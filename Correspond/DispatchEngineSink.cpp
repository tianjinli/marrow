#include "ControlPacket.h"
#include "DispatchEngineSink.h"
#include "ServiceUnits.h"

//////////////////////////////////////////////////////////////////////////////////
// 定时器定义

#define IDI_LOAD_PLATFORM_PARAMETER 1 // 加载参数
#ifdef DEBUG_ENABLED
#define TIME_LOAD_PLATFORM_PARAMETER 60000 // 时间间隔
#else
#define TIME_LOAD_PLATFORM_PARAMETER 600000 // 时间间隔
#endif

//////////////////////////////////////////////////////////////////////////////////
// 接口查询
VOID* CDispatchEngineSink::QueryInterface(REFGUID Guid, DWORD dwQueryVer) {
  QUERYINTERFACE(ITimerEngineEvent, Guid, dwQueryVer);
  QUERYINTERFACE(IControlEngineEvent, Guid, dwQueryVer);
  QUERYINTERFACE(ITCPSocketEvent, Guid, dwQueryVer);
  QUERYINTERFACE(IDataBaseEngineEvent, Guid, dwQueryVer);
  QUERYINTERFACE(ITCPNetworkEngineEvent, Guid, dwQueryVer);
  QUERYINTERFACE(IDispatchEngineSink, Guid, dwQueryVer);
  QUERYINTERFACE_IUNKNOWNEX(IDispatchEngineSink, Guid, dwQueryVer);
  return nullptr;
}

// 启动事件
bool CDispatchEngineSink::OnAttemperEngineStart(IUnknownEx* pIUnknownEx) {
  // 绑定参数
  bind_parameter_ = new tagBindParameter[init_parameter_->max_connect_];
  ZeroMemory(bind_parameter_, sizeof(tagBindParameter) * init_parameter_->max_connect_);

  // 设置定时器
  timer_engine_->SetTimer(IDI_LOAD_PLATFORM_PARAMETER, TIME_LOAD_PLATFORM_PARAMETER, TIMES_INFINITY, 0);
  return true;
}

// 停止事件
bool CDispatchEngineSink::OnAttemperEngineConclude(IUnknownEx* pIUnknownEx) {
  // 状态变量
  collect_item_ = INVALID_WORD;
  wait_collect_item_array_.clear();

  // 设置变量
  timer_engine_ = nullptr;
  network_engine_ = nullptr;

  // 删除数据
  SafeDeleteArray(bind_parameter_);

  // 设置组件
  global_info_manager_.ResetData();

  return true;
}

// 控制事件
bool CDispatchEngineSink::OnEventControl(WORD wIdentifier, VOID* pData, WORD wDataSize) { return false; }

// 调度事件
bool CDispatchEngineSink::OnEventAttemperData(WORD wRequestID, VOID* pData, WORD wDataSize) { return false; }

// 时间事件
bool CDispatchEngineSink::OnEventTimer(DWORD dwTimerID, WPARAM wBindParam) {
  // 加载参数
  if (IDI_LOAD_PLATFORM_PARAMETER == dwTimerID) {
    // 发送通知
    network_engine_->SendDataBatch(MDM_CS_MANAGER_SERVICE, SUB_CS_S_PLATFORM_PARAMETER, 0L, 0, 0);

    return true;
  }

  return false;
}

// 应答事件
bool CDispatchEngineSink::OnEventTCPNetworkBind(DWORD dwSocketID, DWORD dwClientAddr) {
  // 获取索引
  ASSERT(LOWORD(dwSocketID) < init_parameter_->max_connect_);
  if (LOWORD(dwSocketID) >= init_parameter_->max_connect_)
    return false;

  // 变量定义
  WORD wBindIndex = LOWORD(dwSocketID);
  tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

  // 设置变量
  pBindParameter->dwSocketID = dwSocketID;
  pBindParameter->dwClientAddr = dwClientAddr;
  pBindParameter->dwActiveTime = std::time(nullptr);

  return true;
}

// 读取事件
bool CDispatchEngineSink::OnEventTCPNetworkRead(DWORD dwSocketID, TCP_Command Command, VOID* pData, WORD wDataSize) {
  switch (Command.wMainCmdID) {
    case MDM_CS_REGISTER: // 服务注册
    {
      return OnTCPNetworkMainRegister(Command.wSubCmdID, pData, wDataSize, dwSocketID);
    }
    case MDM_CS_SERVICE_INFO: // 服务信息
    {
      return OnTCPNetworkMainServiceInfo(Command.wSubCmdID, pData, wDataSize, dwSocketID);
    }
    case MDM_CS_USER_COLLECT: // 用户命令
    {
      return OnTCPNetworkMainUserCollect(Command.wSubCmdID, pData, wDataSize, dwSocketID);
    }
    case MDM_CS_REMOTE_SERVICE: // 远程服务
    {
      return OnTCPNetworkMainRemoteService(Command.wSubCmdID, pData, wDataSize, dwSocketID);
    }
    case MDM_CS_MANAGER_SERVICE: // 管理服务
    {
      return OnTCPNetworkMainManagerService(Command.wSubCmdID, pData, wDataSize, dwSocketID);
    }
    case MDM_CS_ANDROID_SERVICE: // 机器服务
    {
      return OnTCPNetworkMainAndroidService(Command.wSubCmdID, pData, wDataSize, dwSocketID);
    }
  }

  return false;
}

// 关闭事件
bool CDispatchEngineSink::OnEventTCPNetworkShut(DWORD dwSocketID, DWORD dwClientAddr, DWORD dwActiveTime) {
  // 获取信息
  WORD wBindIndex = LOWORD(dwSocketID);
  tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

  // 游戏服务
  if (pBindParameter->ServiceKind == ServiceKind_Game) {
    // 变量定义
    WORD wBindIndex = LOWORD(dwSocketID);

    // 汇总处理
    if (wBindIndex == collect_item_) {
      // 设置变量
      collect_item_ = INVALID_WORD;

      // 汇总切换
      if (wait_collect_item_array_.size() > 0) {
        // 提取变量
        // INT_PTR nWaitCount = wait_collect_item_array_.size();
        // collect_item_ = wait_collect_item_array_[nWaitCount - 1];
        collect_item_ = wait_collect_item_array_.back();

        // 删除数组
        wait_collect_item_array_.pop_back();
        // wait_collect_item_array_.erase(wait_collect_item_array_.begin() + (nWaitCount - 1));

        // 发送消息
        DWORD socket_id = (bind_parameter_ + collect_item_)->dwSocketID;
        network_engine_->SendData(socket_id, MDM_CS_USER_COLLECT, SUB_CS_S_COLLECT_REQUEST);
      }
    } else {
      // 删除等待
      // for (INT_PTR i = 0; i < wait_collect_item_array_.size(); i++) {
      //   if (wBindIndex == wait_collect_item_array_[i]) {
      //     wait_collect_item_array_.erase(wait_collect_item_array_.begin() + i);
      //     break;
      //   }
      // }
      for (auto it = wait_collect_item_array_.begin(); it != wait_collect_item_array_.end(); ++it) {
        if (wBindIndex == (*it)) {
          wait_collect_item_array_.erase(it);
          break;
        }
      }
    }

    // 变量定义
    CMD_CS_S_ServerRemove ServerRemove;
    ZeroMemory(&ServerRemove, sizeof(ServerRemove));

    // 删除通知
    ServerRemove.wServerID = pBindParameter->wServiceID;
    network_engine_->SendDataBatch(MDM_CS_SERVICE_INFO, SUB_CS_S_SERVER_REMOVE, 0L, &ServerRemove, sizeof(ServerRemove));

    // 注销房间
    global_info_manager_.DeleteServerItem(pBindParameter->wServiceID);
  }

  // 广场服务
  if (pBindParameter->ServiceKind == ServiceKind_Plaza) {
    // 变量定义
    WORD wBindIndex = LOWORD(dwSocketID);
    tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

    // 注销房间
    global_info_manager_.DeletePlazaItem(pBindParameter->wServiceID);
  }

  // 聊天服务
  if (pBindParameter->ServiceKind == ServiceKind_Chat) {
    // 变量定义
    WORD wBindIndex = LOWORD(dwSocketID);
    tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

    // 注销房间
    global_info_manager_.DeleteChatItem(pBindParameter->wServiceID);
  }

  // 清除信息
  ZeroMemory(pBindParameter, sizeof(tagBindParameter));

  return false;
}

// 数据库事件
bool CDispatchEngineSink::OnEventDataBase(WORD wRequestID, DWORD dwContextID, VOID* pData, WORD wDataSize) { return false; }

// 关闭事件
bool CDispatchEngineSink::OnEventTCPSocketShut(WORD wServiceID, BYTE cbShutReason) { return false; }

// 连接事件
bool CDispatchEngineSink::OnEventTCPSocketLink(WORD wServiceID, INT nErrorCode) { return false; }

// 读取事件
bool CDispatchEngineSink::OnEventTCPSocketRead(WORD wServiceID, TCP_Command Command, VOID* pData, WORD wDataSize) { return true; }

// 注册服务
bool CDispatchEngineSink::OnTCPNetworkMainRegister(WORD wSubCmdID, VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  switch (wSubCmdID) {
    case SUB_CS_C_REGISTER_PLAZA: // 注册广场
    {
      // 效验数据
      ASSERT(wDataSize == sizeof(CMD_CS_C_RegisterPlaza));
      if (wDataSize != sizeof(CMD_CS_C_RegisterPlaza))
        return false;

      // 消息定义
      CMD_CS_C_RegisterPlaza* pRegisterPlaza = (CMD_CS_C_RegisterPlaza*) pData;

      // 有效判断
      if ((pRegisterPlaza->szServerName[0] == 0) || (pRegisterPlaza->szServerAddr[0] == 0)) {
        // 变量定义
        CMD_CS_S_RegisterFailure RegisterFailure;
        ZeroMemory(&RegisterFailure, sizeof(RegisterFailure));

        // 设置变量
        RegisterFailure.lErrorCode = 0L;
        LSTRCPYN(RegisterFailure.szDescribeString, TEXT("服务器注册失败，“服务地址”与“服务器名”不合法！"),
                 std::size(RegisterFailure.szDescribeString));

        // 发送消息
        WORD wStringSize = CountStringBuffer(RegisterFailure.szDescribeString);
        WORD wSendSize = sizeof(RegisterFailure) - sizeof(RegisterFailure.szDescribeString) + wStringSize;
        network_engine_->SendData(dwSocketID, MDM_CS_REGISTER, SUB_CS_S_REGISTER_FAILURE, &RegisterFailure, wSendSize);

        // 中断网络
        network_engine_->ShutDownSocket(dwSocketID);

        return true;
      }

      // 设置绑定
      WORD wBindIndex = LOWORD(dwSocketID);
      (bind_parameter_ + wBindIndex)->wServiceID = wBindIndex;
      (bind_parameter_ + wBindIndex)->ServiceKind = ServiceKind_Plaza;

      // 变量定义
      tagGamePlaza GamePlaza;
      ZeroMemory(&GamePlaza, sizeof(GamePlaza));

      // 构造数据
      GamePlaza.wPlazaID = wBindIndex;
      LSTRCPYN(GamePlaza.szServerName, pRegisterPlaza->szServerName, std::size(GamePlaza.szServerName));
      LSTRCPYN(GamePlaza.szServerAddr, pRegisterPlaza->szServerAddr, std::size(GamePlaza.szServerAddr));

      // 注册房间
      global_info_manager_.ActivePlazaItem(wBindIndex, GamePlaza);

      // 发送列表
      SendServerListItem(dwSocketID);

      SendMatchListItem(dwSocketID);

      // 群发设置
      network_engine_->AllowBatchSend(dwSocketID, true, 0);

      return true;
    }
    case SUB_CS_C_REGISTER_SERVER: // 注册房间
    {
      // 效验数据
      ASSERT(wDataSize == sizeof(CMD_CS_C_RegisterServer));
      if (wDataSize != sizeof(CMD_CS_C_RegisterServer))
        return false;

      // 消息定义
      CMD_CS_C_RegisterServer* pRegisterServer = (CMD_CS_C_RegisterServer*) pData;

      // 查找房间
      if (global_info_manager_.SearchServerItem(pRegisterServer->wServerID) != nullptr) {
        // 变量定义
        CMD_CS_S_RegisterFailure RegisterFailure;
        ZeroMemory(&RegisterFailure, sizeof(RegisterFailure));

        // 设置变量
        RegisterFailure.lErrorCode = 0L;
        LSTRCPYN(RegisterFailure.szDescribeString, TEXT("已经存在相同标识的游戏房间服务，房间服务注册失败"),
                 std::size(RegisterFailure.szDescribeString));

        // 发送消息
        WORD wStringSize = CountStringBuffer(RegisterFailure.szDescribeString);
        WORD wSendSize = sizeof(RegisterFailure) - sizeof(RegisterFailure.szDescribeString) + wStringSize;
        network_engine_->SendData(dwSocketID, MDM_CS_REGISTER, SUB_CS_S_REGISTER_FAILURE, &RegisterFailure, wSendSize);

        // 中断网络
        network_engine_->ShutDownSocket(dwSocketID);

        return true;
      }

      // 设置绑定
      WORD wBindIndex = LOWORD(dwSocketID);
      (bind_parameter_ + wBindIndex)->ServiceKind = ServiceKind_Game;
      (bind_parameter_ + wBindIndex)->wServiceID = pRegisterServer->wServerID;

      // 变量定义
      tagGameServer GameServer;
      ZeroMemory(&GameServer, sizeof(GameServer));

      // 构造数据
      GameServer.wKindID = pRegisterServer->wKindID;
      GameServer.wNodeID = pRegisterServer->wNodeID;
      GameServer.wSortID = pRegisterServer->wSortID;
      GameServer.wServerID = pRegisterServer->wServerID;
      GameServer.wServerKind = pRegisterServer->wServerKind;
      GameServer.wServerType = pRegisterServer->wServerType;
      GameServer.wServerLevel = pRegisterServer->wServerLevel;
      GameServer.wServerPort = pRegisterServer->wServerPort;
      GameServer.lCellScore = pRegisterServer->lCellScore;
      GameServer.cbEnterMember = pRegisterServer->cbEnterMember;
      GameServer.lEnterScore = pRegisterServer->lEnterScore;
      GameServer.dwServerRule = pRegisterServer->dwServerRule;
      GameServer.dwOnLineCount = pRegisterServer->dwOnLineCount;
      GameServer.dwFullCount = pRegisterServer->dwFullCount;
      GameServer.wTableCount = pRegisterServer->wTableCount;
      GameServer.dwSetPlayerCount = pRegisterServer->dwSetPlayerCount;

      LSTRCPYN(GameServer.szServerName, pRegisterServer->szServerName, std::size(GameServer.szServerName));
      LSTRCPYN(GameServer.szServerAddr, pRegisterServer->szServerAddr, std::size(GameServer.szServerAddr));
      LSTRCPYN(GameServer.szGameInfomation, pRegisterServer->szGameInfomation, std::size(GameServer.szGameInfomation));
      // 注册房间
      global_info_manager_.ActiveServerItem(wBindIndex, GameServer);

      // 群发房间
      network_engine_->SendDataBatch(MDM_CS_SERVICE_INFO, SUB_CS_S_SERVER_INSERT, 0L, &GameServer, sizeof(GameServer));

      // 发送列表
      SendServerListItem(dwSocketID);

      SendMatchListItem(dwSocketID);

      // 群发设置
      network_engine_->AllowBatchSend(dwSocketID, true, 0L);

      // 汇总通知
      if (collect_item_ == INVALID_WORD) {
        collect_item_ = wBindIndex;
        network_engine_->SendData(dwSocketID, MDM_CS_USER_COLLECT, SUB_CS_S_COLLECT_REQUEST);
      } else
        wait_collect_item_array_.emplace_back(wBindIndex);

      return true;
    }
    case SUB_CS_C_REGISTER_MATCH: // 注册比赛
    {
      // 效验数据
      ASSERT(wDataSize == sizeof(CMD_CS_C_RegisterMatch));
      if (wDataSize != sizeof(CMD_CS_C_RegisterMatch))
        return false;

      // 消息定义
      CMD_CS_C_RegisterMatch* pRegisterMatch = (CMD_CS_C_RegisterMatch*) pData;

      // 查找房间
      CGlobalServerItem* pGlobalServerItem = global_info_manager_.SearchServerItem(pRegisterMatch->wServerID);
      if (pGlobalServerItem == nullptr)
        return true;

      // 变量定义
      tagGameMatch GameMatch;
      ZeroMemory(&GameMatch, sizeof(GameMatch));

      // 构造数据
      GameMatch.wServerID = pRegisterMatch->wServerID;
      GameMatch.dwMatchID = pRegisterMatch->dwMatchID;
      GameMatch.cbMatchType = pRegisterMatch->cbMatchType;
      LSTRCPYN(GameMatch.szMatchName, pRegisterMatch->szMatchName, std::size(GameMatch.szMatchName));

      GameMatch.cbFeeType = pRegisterMatch->cbFeeType;
      GameMatch.cbDeductArea = pRegisterMatch->cbDeductArea;
      GameMatch.lSignupFee = pRegisterMatch->lSignupFee;
      GameMatch.cbSignupMode = pRegisterMatch->cbSignupMode;
      GameMatch.cbJoinCondition = pRegisterMatch->cbJoinCondition;
      GameMatch.cbMemberOrder = pRegisterMatch->cbMemberOrder;
      GameMatch.lExperience = pRegisterMatch->dwExperience;

      GameMatch.cbRankingMode = pRegisterMatch->cbRankingMode;
      GameMatch.wCountInnings = pRegisterMatch->wCountInnings;
      GameMatch.cbFilterGradesMode = pRegisterMatch->cbFilterGradesMode;

      CopyMemory(&GameMatch.cbMatchRule, &pRegisterMatch->cbMatchRule, sizeof(GameMatch.cbMatchRule));

      GameMatch.wRewardCount = pRegisterMatch->wRewardCount;
      CopyMemory(&GameMatch.MatchRewardInfo, pRegisterMatch->MatchRewardInfo, sizeof(GameMatch.MatchRewardInfo));

      // 拷贝数据
      CopyMemory(&pGlobalServerItem->m_GameMatch, &GameMatch, sizeof(pGlobalServerItem->m_GameMatch));

      // 群发房间
      network_engine_->SendDataBatch(MDM_CS_SERVICE_INFO, SUB_CS_S_MATCH_INSERT, 0L, &GameMatch, sizeof(GameMatch));

      return true;
    }
    case SUB_CS_C_REGISTER_CHAT: // 注册聊天
    {
      // 设置绑定
      WORD wBindIndex = LOWORD(dwSocketID);
      (bind_parameter_ + wBindIndex)->wServiceID = wBindIndex;
      (bind_parameter_ + wBindIndex)->ServiceKind = ServiceKind_Chat;

      tagChatServer ChatServer;
      ChatServer.wChatID = wBindIndex;
      ChatServer.dwClientAddr = (bind_parameter_ + wBindIndex)->dwClientAddr;
      ChatServer.dwSocketID = dwSocketID;

      global_info_manager_.ActiveChatItem(wBindIndex, ChatServer);

      // 群发房间
      network_engine_->SendDataBatch(MDM_CS_SERVICE_INFO, SUB_CS_S_CHAT_INSERT, 0L, &ChatServer, sizeof(ChatServer));

      // 发送信息
      network_engine_->SendData(dwSocketID, MDM_CS_SERVICE_INFO, SUB_CS_S_CHAT_INSERT);

      // 群发设置
      network_engine_->AllowBatchSend(dwSocketID, true, 0L);

      return true;
    }
  }

  return false;
}

// 服务状态
bool CDispatchEngineSink::OnTCPNetworkMainServiceInfo(WORD wSubCmdID, VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  switch (wSubCmdID) {
    case SUB_CS_C_SERVER_ONLINE: // 房间人数
    {
      // 效验数据
      ASSERT(wDataSize == sizeof(CMD_CS_C_ServerOnLine));
      if (wDataSize != sizeof(CMD_CS_C_ServerOnLine))
        return false;

      // 消息定义
      CMD_CS_C_ServerOnLine* pServerOnLine = (CMD_CS_C_ServerOnLine*) pData;

      // 获取参数
      WORD wBindIndex = LOWORD(dwSocketID);
      tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

      // 连接效验
      ASSERT(pBindParameter->ServiceKind == ServiceKind_Game);
      if (pBindParameter->ServiceKind != ServiceKind_Game)
        return false;

      // 查找房间
      WORD wServerID = pBindParameter->wServiceID;
      CGlobalServerItem* pGlobalServerItem = global_info_manager_.SearchServerItem(wServerID);

      // 设置人数
      if (pGlobalServerItem != nullptr) {
        // 变量定义
        CMD_CS_S_ServerOnLine ServerOnLine;
        ZeroMemory(&ServerOnLine, sizeof(ServerOnLine));

        // 设置变量
        pGlobalServerItem->m_GameServer.dwAndroidCount = pServerOnLine->dwAndroidCount;
        pGlobalServerItem->m_GameServer.dwOnLineCount = pServerOnLine->dwOnLineCount;
        pGlobalServerItem->m_GameServer.dwSetPlayerCount = pServerOnLine->dwSetCount;
        // 设置变量
        ServerOnLine.wServerID = wServerID;
        ServerOnLine.dwOnLineCount = pServerOnLine->dwOnLineCount;
        ServerOnLine.dwAndroidCount = pServerOnLine->dwAndroidCount;
        ServerOnLine.dwSetCount = pServerOnLine->dwSetCount;
        // 发送通知
        network_engine_->SendDataBatch(MDM_CS_SERVICE_INFO, SUB_CS_S_SERVER_ONLINE, 0L, &ServerOnLine, sizeof(ServerOnLine));
      }

      return true;
    }
    case SUB_CS_C_SERVER_MODIFY: // 房间修改
    {
      // 效验数据
      ASSERT(wDataSize == sizeof(CMD_CS_C_ServerModify));
      if (wDataSize != sizeof(CMD_CS_C_ServerModify))
        return false;

      // 消息定义
      CMD_CS_C_ServerModify* pServerModify = (CMD_CS_C_ServerModify*) pData;

      // 获取参数
      WORD wBindIndex = LOWORD(dwSocketID);
      tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

      // 连接效验
      ASSERT(pBindParameter->ServiceKind == ServiceKind_Game);
      if (pBindParameter->ServiceKind != ServiceKind_Game)
        return false;

      // 查找房间
      ASSERT(global_info_manager_.SearchServerItem(pBindParameter->wServiceID) != nullptr);
      CGlobalServerItem* pGlobalServerItem = global_info_manager_.SearchServerItem(pBindParameter->wServiceID);

      // 房间修改
      if (pGlobalServerItem != nullptr) {
        // 设置变量
        pGlobalServerItem->m_GameServer.wSortID = pServerModify->wSortID;
        pGlobalServerItem->m_GameServer.wKindID = pServerModify->wKindID;
        pGlobalServerItem->m_GameServer.wNodeID = pServerModify->wNodeID;
        pGlobalServerItem->m_GameServer.wServerPort = pServerModify->wServerPort;
        pGlobalServerItem->m_GameServer.dwOnLineCount = pServerModify->dwOnLineCount;
        pGlobalServerItem->m_GameServer.dwAndroidCount = pServerModify->dwAndroidCount;
        pGlobalServerItem->m_GameServer.dwFullCount = pServerModify->dwFullCount;
        pGlobalServerItem->m_GameServer.dwSetPlayerCount = pServerModify->dwSetCount;
        LSTRCPYN(pGlobalServerItem->m_GameServer.szServerName, pServerModify->szServerName, std::size(pGlobalServerItem->m_GameServer.szServerName));
        LSTRCPYN(pGlobalServerItem->m_GameServer.szServerAddr, pServerModify->szServerAddr, std::size(pGlobalServerItem->m_GameServer.szServerAddr));
        LSTRCPYN(pGlobalServerItem->m_GameServer.szGameInfomation, pServerModify->szGameInfomation,
                 std::size(pGlobalServerItem->m_GameServer.szGameInfomation));

        // 变量定义
        CMD_CS_S_ServerModify ServerModify;
        ZeroMemory(&ServerModify, sizeof(ServerModify));

        // 设置变量
        ServerModify.wKindID = pServerModify->wKindID;
        ServerModify.wSortID = pServerModify->wSortID;
        ServerModify.wNodeID = pServerModify->wNodeID;
        ServerModify.wServerID = pBindParameter->wServiceID;
        ServerModify.wServerPort = pServerModify->wServerPort;
        ServerModify.dwOnLineCount = pServerModify->dwOnLineCount;
        ServerModify.dwFullCount = pServerModify->dwFullCount;
        ServerModify.dwAndroidCount = pServerModify->dwAndroidCount;
        LSTRCPYN(ServerModify.szServerAddr, pServerModify->szServerAddr, std::size(ServerModify.szServerAddr));
        LSTRCPYN(ServerModify.szServerName, pServerModify->szServerName, std::size(ServerModify.szServerName));

        // 发送通知
        network_engine_->SendDataBatch(MDM_CS_SERVICE_INFO, SUB_CS_S_SERVER_MODIFY, 0L, &ServerModify, sizeof(ServerModify));
      }

      return true;
    }
  }

  return false;
}

// 用户处理
bool CDispatchEngineSink::OnTCPNetworkMainUserCollect(WORD wSubCmdID, VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  switch (wSubCmdID) {
    case SUB_CS_C_USER_ENTER: // 用户进入
    {
      // 效验数据
      ASSERT(wDataSize == sizeof(CMD_CS_C_UserEnter));
      if (wDataSize != sizeof(CMD_CS_C_UserEnter))
        return false;

      // 消息处理
      CMD_CS_C_UserEnter* pUserEnter = (CMD_CS_C_UserEnter*) pData;
      pUserEnter->szNickName[std::size(pUserEnter->szNickName) - 1] = 0;

      // 获取参数
      WORD wBindIndex = LOWORD(dwSocketID);
      tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

      // 连接效验
      ASSERT(pBindParameter->ServiceKind == ServiceKind_Game);
      if (pBindParameter->ServiceKind != ServiceKind_Game)
        return false;

      // 变量定义
      tagGlobalUserInfo GlobalUserInfo;
      ZeroMemory(&GlobalUserInfo, sizeof(GlobalUserInfo));

      // 设置变量
      GlobalUserInfo.dwUserID = pUserEnter->dwUserID;
      GlobalUserInfo.dwGameID = pUserEnter->dwGameID;
      LSTRCPYN(GlobalUserInfo.szNickName, pUserEnter->szNickName, std::size(GlobalUserInfo.szNickName));

      // 辅助信息
      GlobalUserInfo.cbGender = pUserEnter->cbGender;
      GlobalUserInfo.cbMemberOrder = pUserEnter->cbMemberOrder;
      GlobalUserInfo.cbMasterOrder = pUserEnter->cbMasterOrder;

      // 拷贝详细信息
      memcpy(&GlobalUserInfo.userInfo, &pUserEnter->userInfo, sizeof(tagUserInfo));

      // 激活用户
      global_info_manager_.ActiveUserItem(GlobalUserInfo, pBindParameter->wServiceID);

      CGlobalServerItem* pGlobalServerItem = global_info_manager_.SearchServerItem(pBindParameter->wServiceID);

      // 同步状态
      CMD_CS_S_UserGameStatus UserGameStatus;
      UserGameStatus.dwUserID = pUserEnter->dwUserID;
      UserGameStatus.cbGameStatus = pUserEnter->userInfo.cbUserStatus;
      UserGameStatus.wKindID = pBindParameter->ServiceKind;
      UserGameStatus.wServerID = pBindParameter->wServiceID;
      UserGameStatus.wTableID = INVALID_TABLE;
      UserGameStatus.wChairID = INVALID_CHAIR;
      LSTRCPYN(UserGameStatus.szServerName, pGlobalServerItem->m_GameServer.szServerName, std::size(UserGameStatus.szServerName));

      // 变量定义
      CMapChatID::iterator* posChat = nullptr;

      // 查找房间
      do {
        // 查找房间
        CGlobalChatItem* pGlobalChatItem = global_info_manager_.EnumChatItem(posChat);

        // 终止判断
        if (pGlobalChatItem == nullptr)
          break;

        // 发送状态
        network_engine_->SendData(pGlobalChatItem->m_ChatServer.dwSocketID, MDM_CS_USER_COLLECT, SUB_CS_S_USER_GAMESTATE, &UserGameStatus,
                                  sizeof(UserGameStatus));

      } while (posChat != nullptr);

      return true;
    }
    case SUB_CS_C_USER_LEAVE: // 用户离开
    {
      // 效验数据
      ASSERT(wDataSize == sizeof(CMD_CS_C_UserLeave));
      if (wDataSize != sizeof(CMD_CS_C_UserLeave))
        return false;

      // 消息处理
      CMD_CS_C_UserLeave* pUserLeave = (CMD_CS_C_UserLeave*) pData;

      // 获取参数
      WORD wBindIndex = LOWORD(dwSocketID);
      tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

      // 连接效验
      ASSERT(pBindParameter->ServiceKind == ServiceKind_Game);
      if (pBindParameter->ServiceKind != ServiceKind_Game)
        return false;

      // 删除用户
      global_info_manager_.DeleteUserItem(pUserLeave->dwUserID, pBindParameter->wServiceID);

      CGlobalServerItem* pGlobalServerItem = global_info_manager_.SearchServerItem(pBindParameter->wServiceID);

      // 同步状态
      CMD_CS_S_UserGameStatus UserGameStatus;
      UserGameStatus.dwUserID = pUserLeave->dwUserID;
      UserGameStatus.cbGameStatus = US_NULL;
      UserGameStatus.wKindID = pBindParameter->ServiceKind;
      UserGameStatus.wServerID = pBindParameter->wServiceID;
      UserGameStatus.wTableID = INVALID_TABLE;
      UserGameStatus.wChairID = INVALID_CHAIR;

      LSTRCPYN(UserGameStatus.szServerName, pGlobalServerItem->m_GameServer.szServerName, std::size(UserGameStatus.szServerName));

      CMapChatID::iterator* posChat = nullptr;

      // 查找房间
      do {
        // 查找房间
        CGlobalChatItem* pGlobalChatItem = global_info_manager_.EnumChatItem(posChat);

        // 终止判断
        if (pGlobalChatItem == nullptr)
          break;

        // 发送状态
        network_engine_->SendData(pGlobalChatItem->m_ChatServer.dwSocketID, MDM_CS_USER_COLLECT, SUB_CS_S_USER_GAMESTATE, &UserGameStatus,
                                  sizeof(UserGameStatus));

      } while (posChat != nullptr);

      return true;
    }
    case SUB_CS_C_USER_FINISH: // 用户完成
    {
      // 获取参数
      WORD wBindIndex = LOWORD(dwSocketID);
      tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

      // 连接效验
      ASSERT((collect_item_ == wBindIndex) && (pBindParameter->ServiceKind == ServiceKind_Game));
      if ((collect_item_ != wBindIndex) || (pBindParameter->ServiceKind != ServiceKind_Game))
        return false;

      // 设置变量
      collect_item_ = INVALID_WORD;

      // 汇总切换
      if (wait_collect_item_array_.size() > 0) {
        // 切换汇总
        // INT_PTR nWaitCount = wait_collect_item_array_.size();
        // collect_item_ = wait_collect_item_array_[nWaitCount - 1];
        collect_item_ = wait_collect_item_array_.back();

        // 删除数组
        // wait_collect_item_array_.erase(wait_collect_item_array_.begin() + (nWaitCount - 1));
        wait_collect_item_array_.pop_back();

        // 发送消息
        DWORD socket_id = (bind_parameter_ + collect_item_)->dwSocketID;
        network_engine_->SendData(socket_id, MDM_CS_USER_COLLECT, SUB_CS_S_COLLECT_REQUEST);
      }

      return true;
    }
    case SUB_CS_C_USER_STATUS: // 用户状态
    {
      // 效验数据
      ASSERT(wDataSize == sizeof(CMD_CS_C_UserStatus));
      if (wDataSize != sizeof(CMD_CS_C_UserStatus))
        return false;

      // 消息处理
      CMD_CS_C_UserStatus* pUserStatus = (CMD_CS_C_UserStatus*) pData;

      CGlobalUserItem* pGlobalUserItem = global_info_manager_.SearchUserItemByUserID(pUserStatus->dwUserID);
      if (pGlobalUserItem != nullptr) {
        pGlobalUserItem->UpdateStatus(pUserStatus->wTableID, pUserStatus->wChairID, pUserStatus->cbUserStatus);
      }

      // 获取参数
      WORD wBindIndex = LOWORD(dwSocketID);
      tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

      CGlobalServerItem* pGlobalServerItem = global_info_manager_.SearchServerItem(pBindParameter->wServiceID);

      // 同步状态
      CMD_CS_S_UserGameStatus UserGameStatus;
      UserGameStatus.dwUserID = pUserStatus->dwUserID;
      UserGameStatus.cbGameStatus = pUserStatus->cbUserStatus;
      UserGameStatus.wKindID = pUserStatus->wKindID;
      UserGameStatus.wServerID = pUserStatus->wServerID;
      UserGameStatus.wTableID = pUserStatus->wTableID;
      LSTRCPYN(UserGameStatus.szServerName, pGlobalServerItem->m_GameServer.szServerName, std::size(UserGameStatus.szServerName));

      // 变量定义
      CMapChatID::iterator* posChat = nullptr;

      // 查找房间
      do {
        // 查找房间
        CGlobalChatItem* pGlobalChatItem = global_info_manager_.EnumChatItem(posChat);

        // 终止判断
        if (pGlobalChatItem == nullptr)
          break;

        // 发送状态
        network_engine_->SendData(pGlobalChatItem->m_ChatServer.dwSocketID, MDM_CS_USER_COLLECT, SUB_CS_S_USER_GAMESTATE, &UserGameStatus,
                                  sizeof(UserGameStatus));

      } while (posChat != nullptr);

      return true;
    }
  }

  return false;
}

// 远程服务
bool CDispatchEngineSink::OnTCPNetworkMainRemoteService(WORD wSubCmdID, VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  switch (wSubCmdID) {
    case SUB_CS_C_SEARCH_CORRESPOND: // 协调查找
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_CS_C_SearchCorrespond));
      if (wDataSize != sizeof(CMD_CS_C_SearchCorrespond))
        return false;

      // 处理消息
      CMD_CS_C_SearchCorrespond* pSearchCorrespond = (CMD_CS_C_SearchCorrespond*) pData;
      pSearchCorrespond->szNickName[std::size(pSearchCorrespond->szNickName) - 1] = 0;

      // 变量定义
      CMD_CS_S_SearchCorrespond SearchCorrespond;
      ZeroMemory(&SearchCorrespond, sizeof(SearchCorrespond));

      // 结果用户
      CGlobalUserItem* ResultUserItem[2];
      ZeroMemory(ResultUserItem, sizeof(ResultUserItem));

      // 设置变量
      SearchCorrespond.dwSocketID = pSearchCorrespond->dwSocketID;
      SearchCorrespond.dwClientAddr = pSearchCorrespond->dwClientAddr;

      // 查找用户
      if (pSearchCorrespond->dwGameID != 0L) {
        ResultUserItem[0] = global_info_manager_.SearchUserItemByGameID(pSearchCorrespond->dwGameID);
      }

      // 查找用户
      if (pSearchCorrespond->szNickName[0] != 0) {
        // UTF8 字符串的大写转小写比较复杂且存储时未进行大小写转换故去之
        ResultUserItem[1] = global_info_manager_.SearchUserItemByNickName(pSearchCorrespond->szNickName);
      }

      // 设置结果
      for (BYTE i = 0; i < std::size(ResultUserItem); i++) {
        if (ResultUserItem[i] != nullptr) {
          // 变量定义
          WORD wServerIndex = 0;

          // 查找房间
          do {
            // 查找房间
            CGlobalServerItem* pGlobalServerItem = ResultUserItem[i]->EnumServerItem(wServerIndex++);

            // 终止判断
            if (pGlobalServerItem == nullptr)
              break;
            if (SearchCorrespond.wUserCount >= std::size(SearchCorrespond.UserRemoteInfo))
              break;

            // 索引定义
            WORD wIndex = SearchCorrespond.wUserCount++;

            // 辅助信息
            SearchCorrespond.UserRemoteInfo[wIndex].cbGender = ResultUserItem[i]->GetGender();
            SearchCorrespond.UserRemoteInfo[wIndex].cbMemberOrder = ResultUserItem[i]->GetMemberOrder();
            SearchCorrespond.UserRemoteInfo[wIndex].cbMasterOrder = ResultUserItem[i]->GetMasterOrder();

            // 用户信息
            SearchCorrespond.UserRemoteInfo[wIndex].dwUserID = ResultUserItem[i]->GetUserID();
            SearchCorrespond.UserRemoteInfo[wIndex].dwGameID = ResultUserItem[i]->GetGameID();
            LSTRCPYN(SearchCorrespond.UserRemoteInfo[wIndex].szNickName, ResultUserItem[i]->GetNickName(), LEN_NICKNAME);

            // 房间信息
            SearchCorrespond.UserRemoteInfo[wIndex].wKindID = pGlobalServerItem->GetKindID();
            SearchCorrespond.UserRemoteInfo[wIndex].wServerID = pGlobalServerItem->GetServerID();
            LSTRCPYN(SearchCorrespond.UserRemoteInfo[wIndex].szGameServer, pGlobalServerItem->m_GameServer.szServerName, LEN_SERVER);

            tagUserInfo* pUserInfo = ResultUserItem[i]->GetUserInfo();
            SearchCorrespond.UserRemoteInfo[wIndex].wFaceID = pUserInfo->wFaceID;
            SearchCorrespond.UserRemoteInfo[wIndex].wChairID = pUserInfo->wChairID;
            SearchCorrespond.UserRemoteInfo[wIndex].cbUserStatus = pUserInfo->cbUserStatus;
            SearchCorrespond.UserRemoteInfo[wIndex].wTableID = pUserInfo->wTableID;
            SearchCorrespond.UserRemoteInfo[wIndex].wLastTableID = pUserInfo->wLastTableID;

          } while (true);
        }
      }

      // 发送数据
      WORD wHeadSize = sizeof(SearchCorrespond) - sizeof(SearchCorrespond.UserRemoteInfo);
      WORD wItemSize = sizeof(SearchCorrespond.UserRemoteInfo[0]) * SearchCorrespond.wUserCount;
      network_engine_->SendData(dwSocketID, MDM_CS_REMOTE_SERVICE, SUB_CS_S_SEARCH_CORRESPOND, &SearchCorrespond, wHeadSize + wItemSize);

      return true;
    }
    case SUB_CS_C_SEARCH_ALLCORRESPOND: // 协调查找
    {
      CMD_CS_C_AllSearchCorrespond* pSearchCorrespond = (CMD_CS_C_AllSearchCorrespond*) pData;
      if (pSearchCorrespond->dwCount > 512)
        return false;

      BYTE cbDataBuffer[SOCKET_TCP_PACKET] = {0};
      CMD_CS_S_SearchAllCorrespond* pAllSearchCorrespond = (CMD_CS_S_SearchAllCorrespond*) cbDataBuffer;
      // 设置变量
      pAllSearchCorrespond->dwSocketID = pSearchCorrespond->dwSocketID;
      pAllSearchCorrespond->dwClientAddr = pSearchCorrespond->dwClientAddr;

      int nCount = 0;
      const int cbPacketHeadSize = sizeof(CMD_CS_S_SearchAllCorrespond) - sizeof(tagUserRemoteInfo);
      int cbPacketSize = cbPacketHeadSize;
      // 查找用户
      int nSearchCount = (int) pSearchCorrespond->dwCount;
      for (int i = 0; i < nSearchCount; i++) {
        CGlobalUserItem* pGlobalUserItem = global_info_manager_.SearchUserItemByGameID(pSearchCorrespond->dwGameID[i]);
        if (pGlobalUserItem != nullptr) {
          tagUserInfo* pUserInfo = pGlobalUserItem->GetUserInfo();
          // 查找房间
          WORD wServerIndex = 0;
          CGlobalServerItem* pGlobalServerItem = pGlobalUserItem->EnumServerItem(wServerIndex++);

          pAllSearchCorrespond->UserRemoteInfo[nCount].dwUserID = pUserInfo->dwUserID;
          pAllSearchCorrespond->UserRemoteInfo[nCount].dwGameID = pUserInfo->dwGameID;
          pAllSearchCorrespond->UserRemoteInfo[nCount].cbMasterOrder = pUserInfo->cbMasterOrder;
          pAllSearchCorrespond->UserRemoteInfo[nCount].cbMemberOrder = pUserInfo->cbMemberOrder;
          pAllSearchCorrespond->UserRemoteInfo[nCount].cbGender = pUserInfo->cbGender;
          pAllSearchCorrespond->UserRemoteInfo[nCount].cbUserStatus = pUserInfo->cbUserStatus;
          pAllSearchCorrespond->UserRemoteInfo[nCount].wFaceID = pUserInfo->wFaceID;
          pAllSearchCorrespond->UserRemoteInfo[nCount].wChairID = pUserInfo->wChairID;
          pAllSearchCorrespond->UserRemoteInfo[nCount].wTableID = pUserInfo->wTableID;
          pAllSearchCorrespond->UserRemoteInfo[nCount].wLastTableID = pUserInfo->wLastTableID;
          LSTRCPYN(pAllSearchCorrespond->UserRemoteInfo[nCount].szNickName, pUserInfo->szNickName, LEN_NICKNAME);
          if (pGlobalServerItem != nullptr) {
            pAllSearchCorrespond->UserRemoteInfo[nCount].wServerID = pGlobalServerItem->GetServerID();
            pAllSearchCorrespond->UserRemoteInfo[nCount].wKindID = pGlobalServerItem->GetKindID();
            LSTRCPYN(pAllSearchCorrespond->UserRemoteInfo[nCount].szGameServer, pGlobalServerItem->m_GameServer.szServerName, LEN_SERVER);
          }
          nCount += 1;
          cbPacketSize += sizeof(tagUserRemoteInfo);
        }
        if (cbPacketSize > (SOCKET_TCP_PACKET - sizeof(CMD_CS_S_SearchAllCorrespond))) {
          pAllSearchCorrespond->dwCount = nCount;
          network_engine_->SendData(dwSocketID, MDM_CS_REMOTE_SERVICE, SUB_CS_S_SEARCH_ALLCORRESPOND, pAllSearchCorrespond, cbPacketSize);
          cbPacketSize = cbPacketHeadSize;
          nCount = 0;
        }
      }

      pAllSearchCorrespond->dwCount = nCount;
      if (nCount == 0)
        cbPacketSize = sizeof(CMD_CS_S_SearchAllCorrespond);
      network_engine_->SendData(dwSocketID, MDM_CS_REMOTE_SERVICE, SUB_CS_S_SEARCH_ALLCORRESPOND, pAllSearchCorrespond, cbPacketSize);

      return true;
    }
  }

  return false;
}

// 管理服务
bool CDispatchEngineSink::OnTCPNetworkMainManagerService(WORD wSubCmdID, VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  switch (wSubCmdID) {
    case SUB_CS_C_SYSTEM_MESSAGE: // 系统消息
    {
      // 发送通知
      network_engine_->SendDataBatch(MDM_CS_MANAGER_SERVICE, SUB_CS_S_SYSTEM_MESSAGE, 0L, pData, wDataSize);
      return true;
    }
    case SUB_CS_C_PROPERTY_TRUMPET: // 喇叭消息
    {
      // 发送通知
      network_engine_->SendDataBatch(MDM_CS_MANAGER_SERVICE, SUB_CS_S_PROPERTY_TRUMPET, 0L, pData, wDataSize);

      return true;
    }
  }

  return false;
}

// 机器服务
bool CDispatchEngineSink::OnTCPNetworkMainAndroidService(WORD wSubCmdID, VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  switch (wSubCmdID) {
    case SUB_CS_C_ADDPARAMETER: // 添加参数
    {
      // 参数校验
      ASSERT(sizeof(CMD_CS_C_AddParameter) == wDataSize);
      if (sizeof(CMD_CS_C_AddParameter) != wDataSize)
        return false;

      // 提取数据
      CMD_CS_C_AddParameter* pAddParameter = (CMD_CS_C_AddParameter*) pData;

      // 构造结构
      CMD_CS_S_AddParameter AddParameter;
      CopyMemory(&AddParameter.AndroidParameter, &pAddParameter->AndroidParameter, sizeof(tagAndroidParameter));

      // 发送消息
      SendDataToGame(pAddParameter->wServerID, MDM_CS_ANDROID_SERVICE, SUB_CS_S_ADDPARAMETER, &AddParameter, sizeof(AddParameter));

      return true;
    }
    case SUB_CS_C_MODIFYPARAMETER: // 修改参数
    {
      // 参数校验
      ASSERT(sizeof(CMD_CS_C_ModifyParameter) == wDataSize);
      if (sizeof(CMD_CS_C_ModifyParameter) != wDataSize)
        return false;

      // 提取数据
      CMD_CS_C_ModifyParameter* pModifyParameter = (CMD_CS_C_ModifyParameter*) pData;

      // 构造结构
      CMD_CS_S_ModifyParameter ModifyParameter;
      CopyMemory(&ModifyParameter.AndroidParameter, &pModifyParameter->AndroidParameter, sizeof(tagAndroidParameter));

      // 发送通知
      SendDataToGame(pModifyParameter->wServerID, MDM_CS_ANDROID_SERVICE, SUB_CS_S_MODIFYPARAMETER, &ModifyParameter, sizeof(ModifyParameter));

      return true;
    }
    case SUB_CS_C_DELETEPARAMETER: // 删除参数
    {
      // 参数校验
      ASSERT(sizeof(CMD_CS_C_DeleteParameter) == wDataSize);
      if (sizeof(CMD_CS_C_DeleteParameter) != wDataSize)
        return false;

      // 提取数据
      CMD_CS_C_DeleteParameter* pDeleteParameter = (CMD_CS_C_DeleteParameter*) pData;

      // 构造结构
      CMD_CS_S_DeleteParameter DeleteParameter;
      DeleteParameter.dwBatchID = pDeleteParameter->dwBatchID;

      // 发送通知
      SendDataToGame(pDeleteParameter->wServerID, MDM_CS_ANDROID_SERVICE, SUB_CS_S_DELETEPARAMETER, &DeleteParameter, sizeof(DeleteParameter));

      return true;
    }
  }

  return false;
}

// 发送列表
bool CDispatchEngineSink::SendServerListItem(DWORD dwSocketID) {
  // 变量定义
  WORD wPacketSize = 0L;
  CMapServerID::iterator* Position = nullptr;
  BYTE cbBuffer[SOCKET_TCP_PACKET];

  // 发送信息
  network_engine_->SendData(dwSocketID, MDM_CS_SERVICE_INFO, SUB_CS_S_SERVER_INFO);

  // 收集数据
  do {
    // 发送数据
    if ((wPacketSize + sizeof(tagGameServer)) > sizeof(cbBuffer)) {
      network_engine_->SendData(dwSocketID, MDM_CS_SERVICE_INFO, SUB_CS_S_SERVER_INSERT, cbBuffer, wPacketSize);
      wPacketSize = 0;
    }

    // 获取对象
    tagGameServer* pGameServer = (tagGameServer*) (cbBuffer + wPacketSize);
    CGlobalServerItem* pGlobalServerItem = global_info_manager_.EnumServerItem(Position);

    // 设置数据
    if (pGlobalServerItem != nullptr) {
      wPacketSize += sizeof(tagGameServer);
      CopyMemory(pGameServer, &pGlobalServerItem->m_GameServer, sizeof(tagGameServer));
    }

  } while (Position != nullptr);

  // 发送数据
  if (wPacketSize > 0)
    network_engine_->SendData(dwSocketID, MDM_CS_SERVICE_INFO, SUB_CS_S_SERVER_INSERT, cbBuffer, wPacketSize);

  // 发送完成
  network_engine_->SendData(dwSocketID, MDM_CS_SERVICE_INFO, SUB_CS_S_SERVER_FINISH);

  return true;
}
// 发送列表
bool CDispatchEngineSink::SendMatchListItem(DWORD dwSocketID) {
  // 变量定义
  WORD wPacketSize = 0L;
  CMapServerID::iterator* Position = nullptr;
  BYTE cbBuffer[SOCKET_TCP_PACKET];

  // 收集数据
  do {
    // 发送数据
    if ((wPacketSize + sizeof(tagGameServer)) > sizeof(cbBuffer)) {
      network_engine_->SendData(dwSocketID, MDM_CS_SERVICE_INFO, SUB_CS_S_MATCH_INSERT, cbBuffer, wPacketSize);
      wPacketSize = 0;
    }

    // 获取对象
    tagGameMatch* pGameMatch = (tagGameMatch*) (cbBuffer + wPacketSize);
    CGlobalServerItem* pGlobalServerItem = global_info_manager_.EnumServerItem(Position);

    // 设置数据
    if (pGlobalServerItem != nullptr && pGlobalServerItem->IsMatchServer()) {
      wPacketSize += sizeof(tagGameMatch);
      CopyMemory(pGameMatch, &pGlobalServerItem->m_GameMatch, sizeof(tagGameMatch));
    }

  } while (Position != nullptr);

  // 发送数据
  if (wPacketSize > 0)
    network_engine_->SendData(dwSocketID, MDM_CS_SERVICE_INFO, SUB_CS_S_MATCH_INSERT, cbBuffer, wPacketSize);

  return true;
}

// 房间发送
bool CDispatchEngineSink::SendDataToGame(WORD wServerID, WORD wMainCmdID, WORD wSubCmdID, VOID* pData, WORD wDataSize) {
  // 查找房间
  CGlobalServerItem* pGlobalServerItem = global_info_manager_.SearchServerItem(wServerID);
  if (pGlobalServerItem == nullptr)
    return false;

  // 获取参数
  WORD wBindIndex = pGlobalServerItem->GetIndex();
  tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

  // 发送数据
  DWORD dwSocketID = pBindParameter->dwSocketID;
  network_engine_->SendData(dwSocketID, wMainCmdID, wSubCmdID, pData, wDataSize);

  return true;
}

// 用户发送
bool CDispatchEngineSink::SendDataToUser(WORD wServerID, DWORD dwUserID, WORD wMainCmdID, WORD wSubCmdID, VOID* pData, WORD wDataSize) {
  return true;
}

//////////////////////////////////////////////////////////////////////////////////
