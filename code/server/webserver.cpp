#include "webserver.h"

WebServer::WebServer(int port, int trigMode, int timeoutMS, bool OptLinger, 
        int sqlPort, const char* sqlUser, const  char* sqlPwd, 
        const char* dbName, int connPoolNum, int threadNum,
        bool openLog, int logLevel, int logQueSize):
        port_(port), openLinger_(OptLinger), timeoutMS_(timeoutMS), isClose_(false),
        timer_(new HeapTimer()), threadpool_(new ThreadPool(threadNum)), epoller_(new Epoller()) {
    srcDir_ = getcwd(nullptr, 256);
    assert(srcDir_);
    strncat(srcDir_, "/resources/", 12);
    HttpConn::userCount = 0;
    HttpConn::srcDir = srcDir_;
    SqlConnPool::Instance()->Init("localhost", sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);

    InitEventMode_(trigMode);
    if(!InitSocket_()) isClose_ = true;

    if(openLog) {
        Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
        if(isClose_) {LOG_ERROR("======== Server Init Error ========");}
        else {
            LOG_INFO("======== Server Init ========");
            LOG_INFO("Port:%d, OpenLinger: %s", port_, OptLinger ? "true" : "false");
            LOG_INFO("Listen Mode: %s, Openconn Mode: %s", (listenEvent_ & EPOLLET ? "ET" : "LT"), (connEvent_ & EPOLLET ? "ET" : "LT"));
            LOG_INFO("Logsys level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpConn::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }
}

WebServer::~WebServer() {
    close(listenFd_);
    isClose_ = true;
    free(srcDir_);
    SqlConnPool::Instance()->ClosePool();
}

void WebServer::InitEventMode_(int trigMode) {
    listenEvent_ = EPOLLRDHUP;
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;
    switch(trigMode) {
    case 0:
        break;
    case 1:
        listenEvent_ |= EPOLLET;
        break;
    case 2:
        connEvent_ |= EPOLLET;
        break;
    case 3:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
    default:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    }
    HttpConn::isET = connEvent_ & EPOLLET;
}

void WebServer::Start() {
    int timeMS = -1;
    if(!isClose_) LOG_INFO("======== Server Start ========");
    while(!isClose_) {
        if(timeoutMS_ > 0) {
            timeMS = timer_->GetNextTick();
        }
        int eventCnt = epoller_->Wait(timeMS);
        for(int i = 0; i < eventCnt; i++) {
            int fd = epoller_->GetEventFd(i);
            uint32_t events = epoller_->GetEvents(i);
            if(fd == listenFd_) {
                DealListen_();
            }
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLIN)) {
                assert(users_.contains(fd));
                CloseConn_(&users_[fd]);
            }
            else if(events & EPOLLIN) {
                assert(users_.contains(fd));
                DealRead_(&users_[fd]);
            }
            else if(events & EPOLLOUT) {
                assert(users_.contains(fd));
                DealWrite_(&users_[fd]);
            }
            else {
                LOG_ERROR("Unexpected Event");
            }
        }
    }
}

bool WebServer::InitSocket_() {
    int ret;
    sockaddr_in addr;
    if(port_ > 65535 || port_ < 1024) {
        LOG_ERROR("Unvalid Port: %d", port_);
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htonl(port_);
    linger optLinger = { 0 };
    if(openLinger_) {
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }

    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd_ < 0) {
        LOG_ERROR("Listen Socket Init Failed");
        return false;
    }

    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0) {
        close(listenFd_);
        LOG_ERROR("Listen Socket Options Set Failed");
        return false;
    }

    int optival = 1;
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optival, sizeof(int));
    if(ret < 0) {
        close(listenFd_);
        LOG_ERROR("Listen Socket Reuse Addr Set Failed");
        return false;
    }

    ret = bind(listenFd_, (sockaddr*)&addr, sizeof(addr));
    if(ret < 0) {
        close(listenFd_);
        LOG_ERROR("Listen Socket Bind Failed");
        return false;
    }

    ret = listen(listenFd_, 6);
    if(ret < 0) {
        close(listenFd_);
        LOG_ERROR("Add Listen Failed");
        return false;
    }
    ret = epoller_->AddFd(listenFd_, listenEvent_ | EPOLLIN);
    if(ret == 0) {
        close(listenFd_);
        LOG_ERROR("Add Fd Failed");
        return false;
    }
    SetFdNonblock(listenFd_);
    LOG_INFO("Init Socket Success  Port:%d", port_);
    return true;
}


void WebServer::AddClient_(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users_[fd].init(fd, addr);
    if(timeoutMS_ > 0) {
        timer_->add(fd, timeoutMS_, std::bind(&WebServer::CloseConn_, this, &users_[fd]));
    }
    epoller_->AddFd(fd, connEvent_ | EPOLLIN);
    SetFdNonblock(fd);
    LOG_INFO("New Client[%d] in", users_[fd].GetFd());
}

void WebServer::DealListen_() {
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listenFd_, (sockaddr*)&addr, &len);
        if(fd <= 0) return;
        else if(HttpConn::userCount >= MAX_FD) {
            SendError_(fd, "Server busy");
            LOG_WARN("No Spare Client");
            return;
        }
        AddClient_(fd, addr);
    }while(listenEvent_ & EPOLLET);
}

void WebServer::DealRead_(HttpConn* client) {
    assert(client);
    ExtentTime_(client);
    threadpool_->AddTask(std::bind(&WebServer::OnRead_, this, client));
}

void WebServer::DealWrite_(HttpConn* client) {
    assert(client);
    ExtentTime_(client);
    threadpool_->AddTask(std::bind(&WebServer::OnWrite_, this, client));
}

void WebServer::ExtentTime_(HttpConn* client) {
    assert(client);
    if(timeoutMS_ > 0) {
        timer_->adjust(client->GetFd(), timeoutMS_);
    }
}

void WebServer::CloseConn_(HttpConn* client) {
    assert(client);
    LOG_INFO("Client[%d] quit!", client->GetFd());
    timer_->doWork(client->GetFd());
    client->Close();
    epoller_->DelFd(client->GetFd());
}

void WebServer::SendError_(int fd, const char* info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0) {
        LOG_WARN("Send Error to Client[%d] Failed", fd);
    }
    close(fd);
}

void WebServer::OnRead_(HttpConn* client) {
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);
    if(ret <= 0 && readErrno != EAGAIN) {
        CloseConn_(client);
        return;
    }
    OnProcess(client);
}

void WebServer::OnWrite_(HttpConn* client) {
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if(client->ToWriteBytes() == 0) {
        if(client->IsKeepAlive()) {
            OnProcess(client);
            return;
        }
    }
    else if(ret < 0) {
        if(writeErrno == EAGAIN) {
            epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
            return;
        }
    }
    CloseConn_(client);
}

void WebServer::OnProcess(HttpConn* client) {
    assert(client);
    if(client->process()) {
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
    } else {
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLIN);
    }
}

int WebServer::SetFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}