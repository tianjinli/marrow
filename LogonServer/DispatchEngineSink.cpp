#include "DispatchEngineSink.h"
#include "ControlPacket.h"
#include "ServiceUnits.h"

//////////////////////////////////////////////////////////////////////////////////

// 时间标识
#define IDI_LOAD_GAME_LIST 1 // 加载列表
#define IDI_CONNECT_CORRESPOND 2 // 重连标识
#define IDI_COLLECT_ONLINE_INFO 3 // 统计在线
#define IDI_CONNECT_PERSONAL_ROOM_CORRESPOND 4 // 约战服务器重连标识

// 时间定义
#define TIME_VALID_VERIFY_CODE 0 // 请求间隔
#ifdef _WIN32
#define PERSONAL_SERVICE TEXT("PersonalService.dll")
#else
#define PERSONAL_SERVICE TEXT("PersonalService.so")
#endif
//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CDispatchEngineSink::CDispatchEngineSink() {
  // 清理数组
  m_VerifyCodeArray.RemoveAll();

  DYNLIB_HANDLE hInstLibrary = DYNLIB_LOAD(PERSONAL_SERVICE);
  if (hInstLibrary != nullptr) {
    m_bHasPrsnlRM = true;
    DYNLIB_UNLOAD(hInstLibrary);
  }
}

// 析构函数
CDispatchEngineSink::~CDispatchEngineSink() {
  // 清理资源
  for (INT i = 0; i < m_VerifyCodeArray.GetCount(); i++) {
    tagVerifyCode* pValidateCode = m_VerifyCodeArray[i];
    if (pValidateCode != nullptr) {
      SafeDelete(pValidateCode);
    }
    m_VerifyCodeArray.RemoveAt(i);
  }
}

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

  // 设置时间
  ASSERT(timer_engine_ != nullptr);
#ifdef DEBUG_ENABLED
  timer_engine_->SetTimer(IDI_COLLECT_ONLINE_INFO, 20 * 1000L, TIMES_INFINITY, 0);
#else
  timer_engine_->SetTimer(IDI_COLLECT_ONLINE_INFO, init_parameter_->collect_time_ * 1000L, TIMES_INFINITY, 0);
#endif

  // 获取路径
  TCHAR work_dir[MAX_PATH] = TEXT("");
  CWHService::GetWorkDirectory(work_dir, std::size(work_dir));

  // 构造路径
  const auto filename = std::filesystem::path(StringT(work_dir)) / TEXT("VideoOption.ini");

  // 读取配置
  CWHIniData ini_data;
  ini_data.SetIniFilePath(filename);

  // 读取配置
  TCHAR szServerAddr[LEN_SERVER] = TEXT("");
  m_wAVServerPort = ini_data.ReadInt(TEXT("VideoOption"), TEXT("ServerPort"), 0);
  ini_data.ReadString(TEXT("VideoOption"), TEXT("ServerAddr"), TEXT(""), szServerAddr, LEN_SERVER);
  std::string strServerAddr = ToSimpleUtf8(szServerAddr);
  inet_pton(AF_INET, strServerAddr.c_str(), &m_dwAVServerAddr);

  return true;
}

// 停止事件
bool CDispatchEngineSink::OnAttemperEngineConclude(IUnknownEx* pIUnknownEx) {
  // 状态变量timer_engine_
  m_bNeekCorrespond = true;
  m_bNeekPersonal = true;

  // 组件变量
  timer_engine_ = nullptr;
  database_engine_ = nullptr;
  network_engine_ = nullptr;
  correspond_service_ = nullptr;
  personal_service_ = nullptr;

  // 任务参数
  m_wTaskCount = 0;
  m_wTaskCountBuffer = 0;
  ZeroMemory(m_TaskParameter, sizeof(m_TaskParameter));
  ZeroMemory(m_TaskParameterBuffer, sizeof(m_TaskParameterBuffer));

  // 会员参数
  m_wMemberCount = 0;
  ZeroMemory(m_MemberParameter, sizeof(m_MemberParameter));

  // 等级配置
  m_wLevelCount = 0;
  ZeroMemory(m_GrowLevelConfig, sizeof(m_GrowLevelConfig));

  // 签到配置
  ZeroMemory(m_lCheckInReward, sizeof(m_lCheckInReward));

  // 删除数据
  SafeDeleteArray(bind_parameter_);

  // 列表组件
  m_ServerListManager.ResetServerList();

  // 道具组件
  m_GamePropertyListManager.ResetPropertyListManager();

  return true;
}

// 控制事件
bool CDispatchEngineSink::OnEventControl(WORD wIdentifier, VOID* pData, WORD wDataSize) {
  switch (wIdentifier) {
    case CT_LOAD_DB_GAME_LIST: // 加载列表
    {
      // 加载列表
      m_ServerListManager.DisuseKernelItem();
      database_engine_->PostDataBaseRequest(DBR_GP_LOAD_GAME_LIST, 0, nullptr, 0);

      // 平台参数
      // database_engine_->PostDataBaseRequest(DBR_GP_LOAD_PLATFORM_PARAMETER, 0, nullptr, 0);

      // 加载签到
      // database_engine_->PostDataBaseRequest(DBR_GP_LOAD_CHECKIN_REWARD, 0, nullptr, 0);

      // 加载任务
      // database_engine_->PostDataBaseRequest(DBR_GP_LOAD_TASK_LIST, 0, nullptr, 0);

      // 加载低保
      // database_engine_->PostDataBaseRequest(DBR_GP_LOAD_BASEENSURE, 0, nullptr, 0);

      // 会员参数
      // database_engine_->PostDataBaseRequest(DBR_GP_LOAD_MEMBER_PARAMETER, 0, nullptr, 0);

      // 成长配置
      // database_engine_->PostDataBaseRequest(DBR_GP_LOAD_GROWLEVEL_CONFIG, 0, nullptr, 0);

      // 道具配置
      m_GamePropertyListManager.DisusePropertyTypeItem();
      m_GamePropertyListManager.DisusePropertyItem();
      // database_engine_->PostDataBaseRequest(DBR_GP_LOAD_GAME_PROPERTY_LIST, 0, nullptr, 0);

      // 加载认证
      // database_engine_->PostDataBaseRequest(DBR_GP_LOAD_REAL_AUTH, 0, nullptr, 0);
      return true;
    }
    case CT_CONNECT_CORRESPOND: // 连接协调
    {
      // 发起连接
      tagAddressInfo* pCorrespondAddress = &init_parameter_->correspond_address_;
      correspond_service_->Connect(pCorrespondAddress->szAddress, init_parameter_->correspond_port_);

      // 构造提示
      CLogger::Info(TEXT("正在连接协调服务器 [ {}:{} ]"), pCorrespondAddress->szAddress, init_parameter_->correspond_port_);

      return true;
    }
    case CT_CONNECT_PERSONAL_ROOM_CORRESPOND: // 连接约战
    {
      if (!m_bHasPrsnlRM) {
        return true;
      }
      // 发起连接
      tagAddressInfo* pPersonalAddress = &init_parameter_->personal_address_;
      // 发送数据
      if (personal_service_) {
        personal_service_->Connect(pPersonalAddress->szAddress, init_parameter_->personal_port_);
      }

      // 构造提示
      CLogger::Info(TEXT("正在连接约战服务器 [ {}:{} ]"), pPersonalAddress->szAddress, init_parameter_->personal_port_);

      return true;
    }
  }

  return false;
}

// 调度事件
bool CDispatchEngineSink::OnEventAttemperData(WORD wRequestID, VOID* pData, WORD wDataSize) {
  return false;
}

// 时间事件
bool CDispatchEngineSink::OnEventTimer(DWORD dwTimerID, WPARAM wBindParam) {
  switch (dwTimerID) {
    case IDI_LOAD_GAME_LIST: // 加载列表
    {
      // 加载列表
      m_ServerListManager.DisuseKernelItem();
      database_engine_->PostDataBaseRequest(DBR_GP_LOAD_GAME_LIST, 0, nullptr, 0);

      return true;
    }
    case IDI_CONNECT_CORRESPOND: // 连接协调
    {
      // 发起连接
      tagAddressInfo* pCorrespondAddress = &init_parameter_->correspond_address_;
      correspond_service_->Connect(pCorrespondAddress->szAddress, init_parameter_->correspond_port_);

      // 构造提示
      CLogger::Info(TEXT("正在连接协调服务器 [ {}:{} ]"), pCorrespondAddress->szAddress, init_parameter_->correspond_port_);

      return true;
    }
    case IDI_CONNECT_PERSONAL_ROOM_CORRESPOND: // 约战服务器重连
    {
      if (!m_bHasPrsnlRM) {
        return true;
      }

      // 发起连接
      tagAddressInfo* pPersonalAddress = &init_parameter_->personal_address_;
      if (personal_service_) {
        personal_service_->Connect(pPersonalAddress->szAddress, init_parameter_->personal_port_);
      }

      // 构造提示
      CLogger::Info(TEXT("正在连接约战服务器 [ {}:{} ]"), pPersonalAddress->szAddress, init_parameter_->personal_port_);

      return true;
    }
    case IDI_COLLECT_ONLINE_INFO: // 统计在线
    {
      // 变量定义
      DBR_GP_OnLineCountInfo OnLineCountInfo;
      ZeroMemory(&OnLineCountInfo, sizeof(OnLineCountInfo));

      // 获取总数
      OnLineCountInfo.dwOnLineCountSum = m_ServerListManager.CollectOnlineInfo(false);
      OnLineCountInfo.dwAndroidCountSum = m_ServerListManager.CollectOnlineInfo(true);
      OnLineCountInfo.dwSetCountSum = m_ServerListManager.CollectSetPlayer(INVALID_WORD);
      // 获取类型
      CKindItemMap::iterator* KindPosition = nullptr;
      do {
        // 获取类型
        CGameKindItem* pGameKindItem = m_ServerListManager.EmunGameKindItem(KindPosition);

        // 设置变量
        if (pGameKindItem != nullptr) {
          WORD wKindIndex = OnLineCountInfo.wKindCount++;
          OnLineCountInfo.OnLineCountKind[wKindIndex].wKindID = pGameKindItem->m_GameKind.wKindID;

          // 目录人数
          OnLineCountInfo.OnLineCountKind[wKindIndex].dwOnLineCount = pGameKindItem->m_GameKind.dwOnLineCount;
          OnLineCountInfo.OnLineCountKind[wKindIndex].dwAndroidCount = pGameKindItem->m_GameKind.dwAndroidCount;
          OnLineCountInfo.OnLineCountKind[wKindIndex].dwSetCount = pGameKindItem->m_GameKind.dwSetCount;
        }

        // 溢出判断
        if (OnLineCountInfo.wKindCount >= std::size(OnLineCountInfo.OnLineCountKind)) {
          ASSERT(FALSE);
          break;
        }

      } while (KindPosition != nullptr);

      // 发送请求
      WORD wHeadSize = sizeof(OnLineCountInfo) - sizeof(OnLineCountInfo.OnLineCountKind);
      WORD wSendSize = wHeadSize + OnLineCountInfo.wKindCount * sizeof(OnLineCountInfo.OnLineCountKind[0]);
      database_engine_->PostDataBaseRequest(DBR_GP_ONLINE_COUNT_INFO, 0, &OnLineCountInfo, wSendSize);

      return true;
    }
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
  pBindParameter->dwActiveTime = (DWORD) time(nullptr);

  return true;
}

// 关闭事件
bool CDispatchEngineSink::OnEventTCPNetworkShut(DWORD dwSocketID, DWORD dwClientAddr, DWORD dwActiveTime) {
  // 清除信息
  WORD wBindIndex = LOWORD(dwSocketID);
  ZeroMemory((bind_parameter_ + wBindIndex), sizeof(tagBindParameter));

  return false;
}

// 读取事件
bool CDispatchEngineSink::OnEventTCPNetworkRead(DWORD dwSocketID, TCP_Command Command, VOID* pData, WORD wDataSize) {
  switch (Command.wMainCmdID) {
    case MDM_GP_LOGON: // 登录命令
    {
      return OnTCPNetworkMainPCLogon(Command.wSubCmdID, pData, wDataSize, dwSocketID);
    }
    case MDM_GP_SERVER_LIST: // 列表命令
    {
      return OnTCPNetworkMainPCServerList(Command.wSubCmdID, pData, wDataSize, dwSocketID);
    }
    case MDM_GP_USER_SERVICE: // 服务命令
    {
      return OnTCPNetworkMainPCUserService(Command.wSubCmdID, pData, wDataSize, dwSocketID);
    }
    case MDM_GP_REMOTE_SERVICE: // 远程服务
    {
      return OnTCPNetworkMainPCRemoteService(Command.wSubCmdID, pData, wDataSize, dwSocketID);
    }
    case MDM_GP_ANDROID_SERVICE: // 机器服务
    {
      return OnTCPNetworkMainAndroidService(Command.wSubCmdID, pData, wDataSize, dwSocketID);
    }
    case MDM_GP_PROPERTY: // 道具服务
    {
      return OnTCPNetworkMainPCProperty(Command.wSubCmdID, pData, wDataSize, dwSocketID);
    }
    case MDM_MB_LOGON: // 登录命令
    {
      return OnTCPNetworkMainMBLogon(Command.wSubCmdID, pData, wDataSize, dwSocketID);
    }
    case MDM_MB_SERVER_LIST: // 列表命令
    {
      return OnTCPNetworkMainMBServerList(Command.wSubCmdID, pData, wDataSize, dwSocketID);
    }
    case MDM_MB_PERSONAL_SERVICE: // 约战房间
    {
      return OnTCPNetworkMainMBPersonalService(Command.wSubCmdID, pData, wDataSize, dwSocketID);
    }
    case MDM_MB_VIDEO_PLAY_BACK_INFO: {
      return OnTCPNetworkMBVideoPlayBackInfo(Command.wSubCmdID, pData, wDataSize, dwSocketID);
    }
  }

  return false;
}

// 数据库事件
bool CDispatchEngineSink::OnEventDataBase(WORD wRequestID, DWORD dwContextID, VOID* pData, WORD wDataSize) {
  switch (wRequestID) {
    case DBO_GP_LOGON_SUCCESS: // 登录成功
    {
      return OnDBPCLogonSuccess(dwContextID, pData, wDataSize);
    }
    case DBO_GP_LOGON_FAILURE: // 登录失败
    {
      return OnDBPCLogonFailure(dwContextID, pData, wDataSize);
    }
    case DBO_GP_VALIDATE_MBCARD: // 校验密保
    {
      return OnDBPCLogonValidateMBCard(dwContextID, pData, wDataSize);
    }
    case DBO_GP_VALIDATE_PASSPORT: // 校验证件
    {
      return OnDBPCLogonValidatePassPort(dwContextID, pData, wDataSize);
    }
    case DBO_GP_VERIFY_RESULT: // 验证结果
    {
      return OnDBPCLogonVerifyResult(dwContextID, pData, wDataSize);
    }
    case DBO_GP_PLATFORM_PARAMETER: // 平台参数
    {
      return OnDBPCPlatformParameter(dwContextID, pData, wDataSize);
    }
    case DBO_GP_USER_FACE_INFO: // 用户头像
    {
      return OnDBPCUserFaceInfo(dwContextID, pData, wDataSize);
    }
    case DBO_GP_USER_INDIVIDUAL: // 用户信息
    {
      return OnDBPCUserIndividual(dwContextID, pData, wDataSize);
    }
    case DBO_GP_BIND_SPREADER_RESULT: // 绑定推广
    {
      return OnDBUserBindSpreaderResult(dwContextID, pData, wDataSize);
    }
    case DBO_GP_USER_INSURE_INFO: // 银行资料
    {
      return OnDBPCUserInsureInfo(dwContextID, pData, wDataSize);
    }
    case DBO_GP_USER_INSURE_SUCCESS: // 银行成功
    {
      return OnDBPCUserInsureSuccess(dwContextID, pData, wDataSize);
    }
    case DBO_GP_USER_INSURE_FAILURE: // 银行失败
    {
      return OnDBPCUserInsureFailure(dwContextID, pData, wDataSize);
    }
    case DBO_GP_USER_INSURE_USER_INFO: // 用户信息
    {
      return OnDBPCUserInsureUserInfo(dwContextID, pData, wDataSize);
    }
    case DBO_GP_USER_INSURE_ENABLE_RESULT: // 开通结果
    {
      return OnDBPCUserInsureEnableResult(dwContextID, pData, wDataSize);
    }
    case DBO_GP_QUERY_TRANSFER_REBATE_RESULT: // 返利结果
    {
      return OnDBPCQueryTransferRebateResult(dwContextID, pData, wDataSize);
    }
    case DBO_GP_ANDROID_PARAMETER: // 机器参数
    {
      return OnDBAndroidParameter(dwContextID, pData, wDataSize);
    }
    case DBO_GP_INDIVIDUAL_RESULT: {
      return OnDBIndividualResult(dwContextID, pData, wDataSize);
    }
    case DBO_GP_OPERATE_SUCCESS: // 操作成功
    {
      return OnDBPCOperateSuccess(dwContextID, pData, wDataSize);
    }
    case DBO_GP_OPERATE_FAILURE: // 操作失败
    {
      return OnDBPCOperateFailure(dwContextID, pData, wDataSize);
    }
    case DBO_GP_GAME_TYPE_ITEM: // 类型子项
    {
      return OnDBPCGameTypeItem(dwContextID, pData, wDataSize);
    }
    case DBO_GP_GAME_KIND_ITEM: // 类型子项
    {
      return OnDBPCGameKindItem(dwContextID, pData, wDataSize);
    }
    case DBO_GP_GAME_NODE_ITEM: // 节点子项
    {
      return OnDBPCGameNodeItem(dwContextID, pData, wDataSize);
    }
    case DBO_GP_GAME_PAGE_ITEM: // 定制子项
    {
      return OnDBPCGamePageItem(dwContextID, pData, wDataSize);
    }
    case DBO_GP_GAME_LIST_RESULT: // 加载结果
    {
      return OnDBPCGameListResult(dwContextID, pData, wDataSize);
    }
    case DBO_GP_CHECKIN_INFO: // 签到信息
    {
      return OnDBPCUserCheckInInfo(dwContextID, pData, wDataSize);
    }
    case DBO_GP_CHECKIN_RESULT: // 签到结果
    {
      return OnDBPCUserCheckInResult(dwContextID, pData, wDataSize);
    }
    case DBO_GP_CHECKIN_REWARD: // 签到奖励
    {
      return OnDBPCCheckInReward(dwContextID, pData, wDataSize);
    }
    case DBO_GP_TASK_LIST: // 任务列表
    {
      return OnDBPCTaskList(dwContextID, pData, wDataSize);
    }
    case DBO_GP_TASK_LIST_END: // 任务结束
    {
      return OnDBPCTaskListEnd(dwContextID, pData, wDataSize);
    }
    case DBO_GP_TASK_INFO: // 任务信息
    {
      return OnDBPCTaskInfo(dwContextID, pData, wDataSize);
    }
    case DBO_GP_TASK_RESULT: // 任务结果
    {
      return OnDBPCTaskResult(dwContextID, pData, wDataSize);
    }
    case DBO_MB_VIDEO_LIST: {
      return OnDBMBVideoList(dwContextID, pData, wDataSize);
    }
    case DBO_MB_VIDEO_LIST_END: {
      return OnDBMBVideoListEnd(dwContextID, pData, wDataSize);
    }
    case DBO_MB_VIDEO_DETAILS: {
      return OnDBMBVideoDetails(dwContextID, pData, wDataSize);
    }
    case DBO_MB_VIDEO_DETAILS_END: {
      return OnDBMBVideoDetailsEnd(dwContextID, pData, wDataSize);
    }
    case DBO_MB_PLAY_BACK_CODE_YZ_RESULT: {
      return OnDBMBPlayBackCodeYZ(dwContextID, pData, wDataSize);
    }
    case DBO_MB_PLAY_BACK_CODE_RESULT: {
      return OnDBMBPlayBackCode(dwContextID, pData, wDataSize);
    }
    case DBO_GP_BASEENSURE_PARAMETER: // 低保参数
    {
      return OnDBPCBaseEnsureParameter(dwContextID, pData, wDataSize);
    }
    case DBO_GP_BASEENSURE_RESULT: // 低保结果
    {
      return OnDBPCBaseEnsureResult(dwContextID, pData, wDataSize);
    }
    case DBO_GP_GROWLEVEL_CONFIG: // 等级配置
    {
      return OnDBPCGrowLevelConfig(dwContextID, pData, wDataSize);
    }
    case DBO_GP_GROWLEVEL_PARAMETER: // 等级参数
    {
      return OnDBPCGrowLevelParameter(dwContextID, pData, wDataSize);
    }
    case DBO_GP_GROWLEVEL_UPGRADE: // 等级升级
    {
      return OnDBPCGrowLevelUpgrade(dwContextID, pData, wDataSize);
    }
    case DBO_GP_GAME_PROPERTY_TYPE_ITEM: // 道具类型
    {
      return OnDBGamePropertyTypeItem(dwContextID, pData, wDataSize);
    }
    case DBO_GP_GAME_PROPERTY_RELAT_ITEM: // 道具关系
    {
      return OnDBGamePropertyRelatItem(dwContextID, pData, wDataSize);
    }
    case DBO_GP_GAME_PROPERTY_ITEM: // 道具节点
    {
      return OnDBGamePropertyItem(dwContextID, pData, wDataSize);
    }
    case DBO_GP_GAME_PROPERTY_SUB_ITEM: // 道具节点
    {
      return OnDBGamePropertySubItem(dwContextID, pData, wDataSize);
    }
    case DBO_GP_GAME_PROPERTY_LIST_RESULT: // 道具结果
    {
      return OnDBGamePropertyListResult(dwContextID, pData, wDataSize);
    }
    case DBO_GP_GAME_PROPERTY_BUY: {
      return OnDBGamePropertyBuy(dwContextID, pData, wDataSize);
    }
    case DBO_GP_GAME_PROPERTY_FAILURE: {
      return OnDBGamePropertyFailure(dwContextID, pData, wDataSize);
    }
    case DBO_GP_QUERY_BACKPACK: // 背包查询
    {
      return OnDBQueryUserBackpack(dwContextID, pData, wDataSize);
    }
    case DBO_GP_PROPERTY_QUERY_SINGLE: // 背包查询
    {
      return OnDBQueryPropertySingle(dwContextID, pData, wDataSize);
    }
    case DBO_GP_USER_PROPERTY_BUFF: // 道具Buff
    {
      return OnDBUserPropertyBuff(dwContextID, pData, wDataSize);
    }
    case DBO_GP_PROPERTY_PRESENT: // 赠送道具
    {
      return OnDBUserPropertyPresent(dwContextID, pData, wDataSize);
    }
    case DBO_GP_GAME_PROPERTY_USE: // 道具使用
    {
      return OnDBGamePropertyUse(dwContextID, pData, wDataSize);
    }
    case DBO_GP_QUERY_SEND_PRESENT: // 查询赠送
    {
      return OnDBQuerySendPresent(dwContextID, pData, wDataSize);
    }
    case DBO_GP_GET_SEND_PRESENT: // 获取赠送
    {
      return OnDBGetSendPresent(dwContextID, pData, wDataSize);
    }
    case DBO_GP_MEMBER_PARAMETER: // 会员参数
    {
      return OnDBPCMemberParameter(dwContextID, pData, wDataSize);
    }
    case DBO_GP_MEMBER_QUERY_INFO_RESULT: // 会员查询
    {
      return OnDBPCMemberDayQueryInfoResult(dwContextID, pData, wDataSize);
    }
    case DBO_GP_MEMBER_DAY_PRESENT_RESULT: // 会员送金
    {
      return OnDBPCMemberDayPresentResult(dwContextID, pData, wDataSize);
    }
    case DBO_GP_MEMBER_DAY_GIFT_RESULT: // 会员礼包
    {
      return OnDBPCMemberDayGiftResult(dwContextID, pData, wDataSize);
    }
    case DBO_GP_PURCHASE_RESULT: // 购买结果
    {
      return OnDBPCPurchaseResult(dwContextID, pData, wDataSize);
    }
    case DBO_GP_EXCHANGE_RESULT: // 兑换结果
    {
      return OnDBPCExChangeResult(dwContextID, pData, wDataSize);
    }
    case DBO_GP_ROOM_CARD_EXCHANGE_RESULT: // 兑换结果
    {
      return OnDBPCExChangeRoomCardToScoreResult(dwContextID, pData, wDataSize);
    }
    case DBO_GP_SPREAD_INFO: // 推广信息
    {
      return OnDBPCUserSpreadInfo(dwContextID, pData, wDataSize);
    }
    case DBO_GP_REAL_AUTH_PARAMETER: // 实名信息
    {
      return OnDBPCRealAuthParameter(dwContextID, pData, wDataSize);
    }
    case DBO_MB_LOGON_SUCCESS: // 登录成功
    {
      return OnDBMBLogonSuccess(dwContextID, pData, wDataSize);
    }
    case DBO_MB_LOGON_FAILURE: // 登录失败
    {
      return OnDBMBLogonFailure(dwContextID, pData, wDataSize);
    }
    case DBO_GP_LOTTERY_CONFIG: // 抽奖配置
    {
      return OnDBPCLotteryConfig(dwContextID, pData, wDataSize);
    }
    case DBO_GP_LOTTERY_USER_INFO: // 抽奖信息
    {
      return OnDBPCLotteryUserInfo(dwContextID, pData, wDataSize);
    }
    case DBO_GP_LOTTERY_RESULT: // 抽奖结果
    {
      return OnDBPCLotteryResult(dwContextID, pData, wDataSize);
    }
    case DBO_GP_QUERY_USER_GAME_DATA: // 游戏数据
    {
      return OnDBPCQueryUserGameData(dwContextID, pData, wDataSize);
    }
    case DBO_GP_AGENT_GAME_KIND_ITEM: {
      return OnDBPCAgentGameList(dwContextID, pData, wDataSize);
    }
    case DBO_MB_AGENT_GAME_KIND_ITEM: {
      return OnDBMBAgentGameList(dwContextID, pData, wDataSize);
    }
    case DBO_MB_PERSONAL_PARAMETER: // 约战房间参数
    {
      return OnDBMBPersonalParameter(dwContextID, pData, wDataSize);
    }
    case DBO_MB_PERSONAL_FEE_LIST: // 约战房间费用列表
    {
      return OnDBMBPersonalFeeList(dwContextID, pData, wDataSize);
    }
    case DBO_MB_PERSONAL_CELL_SCORE_LIST: // 私人房间费用列表
    {
      return OnDBMBPersonalCellScore(dwContextID, pData, wDataSize);
    }
    case DBO_MB_PERSONAL_ROOM_LIST: // 约战房间列表
    {
      return OnDBMBPersonalRoomListInfo(dwContextID, pData, wDataSize);
    }
    case DBO_MB_GET_PERSONAL_RULE: // 私人房间自定义配置
    {
      return OnDBMBPersonalRule(dwContextID, pData, wDataSize);
    }
    case DBO_GR_QUERY_USER_ROOM_SCORE: // 约战房间结算信息
    {
      return OnDBQueryUserRoomScore(dwContextID, pData, wDataSize);
    }
    case DBO_GR_QUERY_PERSONAL_ROOM_USER_INFO_RESULT: // 约战房间结算信息
    {
      return OnDBQueryPersonalRoomUersInfoResult(dwContextID, pData, wDataSize);
    }
    case DBO_MB_GET_PARAMETER_END: // 私人获取结束
    {
      return OnDBQueryPersonalRoomEnd(dwContextID, pData, wDataSize);
    }
    case SUB_GP_CREATE_CLUB_RESULT: {
      network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_CREATE_CLUB_RESULT, pData, wDataSize);
      return true;
    }
    case SUB_GP_CLUB_JOINUSER_LIST: {
      network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_CLUB_JOINUSER_LIST, pData, wDataSize);
      return true;
    }
    case SUB_GP_DEAL_USER_JOIN_CLUB_RESULT: {
      network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_DEAL_USER_JOIN_CLUB_RESULT, pData, wDataSize);
      return true;
    }
    case SUB_GP_CREATE_CLUB_ROOM_RULE_RESULT: {
      network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_CREATE_CLUB_ROOM_RULE_RESULT, pData, wDataSize);
      return true;
    }
    case SUB_GP_MOVE_JEWEL_TO_CLUB_RESULT: {
      network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_MOVE_JEWEL_TO_CLUB_RESULT, pData, wDataSize);
      return true;
    }
    case SUB_GP_CLUB_ALL_ROOM_RULE: {
      network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_CLUB_ALL_ROOM_RULE, pData, wDataSize);
      return true;
    }
    case SUB_GP_DELETE_CLUB_ROOM_RULE_RESULT: {
      network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_DELETE_CLUB_ROOM_RULE_RESULT, pData, wDataSize);
      return true;
    }
    case SUB_GP_DELETE_CLUB_USER_RESULT: {
      network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_DELETE_CLUB_USER_RESULT, pData, wDataSize);
      return true;
    }

    case SUB_GP_QUERY_JOIN_CLUB_RESULT: {
      network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_QUERY_JOIN_CLUB_RESULT, pData, wDataSize);
      return true;
    }

    case SUB_GP_QUERY_QUIT_CLUB_RESULT: {
      network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_QUERY_QUIT_CLUB_RESULT, pData, wDataSize);
      return true;
    }
    case SUB_GP_CLUB_ROOM_LIST: {
      network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_CLUB_ROOM_LIST, pData, wDataSize);
      return true;
    }
    case SUB_GP_CLUB_LIST: {
      network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_CLUB_LIST, pData, wDataSize);
      return true;
    }
    case SUB_GP_CLUB_USER_LIST_LUA: {
      network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_CLUB_USER_LIST_LUA, pData, wDataSize);
      return true;
    }
    case SUB_GP_QUERY_CLUB_NAME_RESULT: {
      network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_QUERY_CLUB_NAME_RESULT, pData, wDataSize);
      return true;
    }
    case SUB_GP_MODIFY_CLUB_SUMMARY_RESULT: {
      network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_MODIFY_CLUB_SUMMARY_RESULT, pData, wDataSize);
      return true;
    }
    case SUB_GP_JOIN_USER_TO_CLUB_RESULT: {
      network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_JOIN_USER_TO_CLUB_RESULT, pData, wDataSize);
      return true;
    }
  }

  return false;
}

// 关闭事件
bool CDispatchEngineSink::OnEventTCPSocketShut(WORD wServiceID, BYTE cbShutReason) {
  // 协调连接
  if (wServiceID == NETWORK_CORRESPOND) {
    // 重连判断
    if (m_bNeekCorrespond == true) {
      // 设置时间
      ASSERT(timer_engine_ != nullptr);
      if (timer_engine_->SetTimer(IDI_CONNECT_CORRESPOND, init_parameter_->connect_time_ * 1000L, 1, 0)) {
        // 构造提示
        CLogger::Warn(TEXT("与协调服务器的连接关闭了， {} 秒后将重新连接"), init_parameter_->connect_time_);
      }

      return true;
    }
  }
  // 约战连接
  else if (wServiceID == NETWORK_PERSONAL) {
    // 重连判断
    if (m_bNeekPersonal) {
      // 设置时间
      ASSERT(timer_engine_ != nullptr);
      if (timer_engine_->SetTimer(IDI_CONNECT_PERSONAL_ROOM_CORRESPOND, init_parameter_->connect_time_ * 1000L, 1, 0)) { // 构造提示
        CLogger::Warn(TEXT("与约战服务器的连接关闭了， {} 秒后将重新连接"), init_parameter_->connect_time_);
      }

      return true;
    }
  }

  return false;
}

// 连接事件
bool CDispatchEngineSink::OnEventTCPSocketLink(WORD wServiceID, INT nErrorCode) {
  // 协调连接
  if (wServiceID == NETWORK_CORRESPOND) {
    // 错误判断
    if (nErrorCode != 0) {
      // 设置时间
      ASSERT(timer_engine_ != nullptr);
      if (timer_engine_->SetTimer(IDI_CONNECT_CORRESPOND, init_parameter_->connect_time_ * 1000L, 1, 0)) {
        // 构造提示
        CLogger::Warn(TEXT("协调服务器连接失败 [ {} ]，{} 秒后将重新连接"), nErrorCode, init_parameter_->connect_time_);
      }
      return false;
    }

    // 提示消息
    CLogger::Info(TEXT("正在注册登录服务器..."));

    // 变量定义
    CMD_CS_C_RegisterPlaza RegisterPlaza;
    ZeroMemory(&RegisterPlaza, sizeof(RegisterPlaza));

    // 设置变量
    LSTRCPYN(RegisterPlaza.szServerName, init_parameter_->server_name_, std::size(RegisterPlaza.szServerName));
    LSTRCPYN(RegisterPlaza.szServerAddr, init_parameter_->service_address_.szAddress, std::size(RegisterPlaza.szServerAddr));

    // 发送数据
    correspond_service_->SendData(MDM_CS_REGISTER, SUB_CS_C_REGISTER_PLAZA, &RegisterPlaza, sizeof(RegisterPlaza));

    return true;
  } else if (wServiceID == NETWORK_PERSONAL) {
    // 错误判断
    if (nErrorCode != 0) {
      // 设置时间
      ASSERT(timer_engine_ != nullptr);
      if (timer_engine_->SetTimer(IDI_CONNECT_PERSONAL_ROOM_CORRESPOND, init_parameter_->connect_time_ * 1000L, 1, 0)) {
        // 构造提示
        CLogger::Warn(TEXT("约战服务器连接失败 [ {} ]，{} 秒后将重新连接"), nErrorCode, init_parameter_->connect_time_);
      }

      return false;
    }
  }

  return true;
}

// 读取事件
bool CDispatchEngineSink::OnEventTCPSocketRead(WORD wServiceID, TCP_Command Command, VOID* pData, WORD wDataSize) {
  // 协调连接
  if (wServiceID == NETWORK_CORRESPOND) {
    switch (Command.wMainCmdID) {
      case MDM_CS_REGISTER: // 注册服务
      {
        return OnTCPSocketMainRegister(Command.wSubCmdID, pData, wDataSize);
      }
      case MDM_CS_SERVICE_INFO: // 服务信息
      {
        return OnTCPSocketMainServiceInfo(Command.wSubCmdID, pData, wDataSize);
      }
      case MDM_CS_REMOTE_SERVICE: // 远程服务
      {
        return OnTCPSocketMainRemoteService(Command.wSubCmdID, pData, wDataSize);
      }
      case MDM_CS_MANAGER_SERVICE: // 管理服务
      {
        return OnTCPSocketMainManagerService(Command.wSubCmdID, pData, wDataSize);
      }
    }
  } else if (wServiceID == NETWORK_PERSONAL) {
    if (wServiceID == MDM_CS_SERVICE_INFO) {
      return OnTCPSocketPersonalServiceInfo(Command.wSubCmdID, pData, wDataSize);
    }

    return true;
  }

  // 错误断言
  ASSERT(FALSE);
  return true;
}

// 注册事件
bool CDispatchEngineSink::OnTCPSocketMainRegister(WORD wSubCmdID, VOID* pData, WORD wDataSize) {
  switch (wSubCmdID) {
    case SUB_CS_S_REGISTER_FAILURE: // 注册失败
    {
      // 变量定义
      CMD_CS_S_RegisterFailure* pRegisterFailure = (CMD_CS_S_RegisterFailure*) pData;

      // 效验参数
      ASSERT(wDataSize >= (sizeof(CMD_CS_S_RegisterFailure) - sizeof(pRegisterFailure->szDescribeString)));
      if (wDataSize < (sizeof(CMD_CS_S_RegisterFailure) - sizeof(pRegisterFailure->szDescribeString)))
        return false;

      // 关闭处理
      m_bNeekCorrespond = false;
      correspond_service_->CloseSocket();

      // 显示消息
      LPCTSTR pszDescribeString = pRegisterFailure->szDescribeString;
      if (StrLenT(pszDescribeString) > 0)
        CLogger::Error(TEXT("{}: {}"), TEXT(__FUNCTION__), pszDescribeString);

      // 事件通知
      service_units->OnCorrespondResult(ER_FAILURE);

      return true;
    }
  }

  return true;
}

// 列表事件
bool CDispatchEngineSink::OnTCPSocketMainServiceInfo(WORD wSubCmdID, VOID* pData, WORD wDataSize) {
  switch (wSubCmdID) {
    case SUB_CS_S_SERVER_INFO: // 房间信息
    {
      // 废弃列表
      m_ServerListManager.DisuseServerItem();

      return true;
    }
    case SUB_CS_S_SERVER_ONLINE: // 房间人数
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_CS_S_ServerOnLine));
      if (wDataSize != sizeof(CMD_CS_S_ServerOnLine))
        return false;

      // 变量定义
      CMD_CS_S_ServerOnLine* pServerOnLine = (CMD_CS_S_ServerOnLine*) pData;

      // 查找房间
      CGameServerItem* pGameServerItem = m_ServerListManager.SearchGameServer(pServerOnLine->wServerID);
      if (pGameServerItem == nullptr)
        return true;

      // 获取对象
      tagGameServer* pGameServer = &pGameServerItem->m_GameServer;

      // 设置人数
      DWORD dwOldOnlineCount = 0, dwOldAndroidCount = 0;
      dwOldOnlineCount = pGameServer->dwOnLineCount;
      dwOldAndroidCount = pGameServer->dwAndroidCount;

      // 房间人数
      pGameServer->dwAndroidCount = pServerOnLine->dwAndroidCount;
      pGameServer->dwOnLineCount = pServerOnLine->dwOnLineCount;

      // 目录人数
      CGameKindItem* pGameKindItem = m_ServerListManager.SearchGameKind(pGameServer->wKindID);
      if (pGameKindItem != nullptr) {
        // 目录总数
        pGameKindItem->m_GameKind.dwOnLineCount -= dwOldOnlineCount;
        pGameKindItem->m_GameKind.dwOnLineCount += pGameServer->dwOnLineCount;

        // 机器人数
        pGameKindItem->m_GameKind.dwAndroidCount -= dwOldAndroidCount;
        pGameKindItem->m_GameKind.dwAndroidCount += pServerOnLine->dwAndroidCount;
      }

      return true;
    }
    case SUB_CS_S_SERVER_INSERT: // 房间插入
    {
      // 效验参数
      ASSERT(wDataSize % sizeof(tagGameServer) == 0);
      if (wDataSize % sizeof(tagGameServer) != 0)
        return false;

      // 变量定义
      WORD wItemCount = wDataSize / sizeof(tagGameServer);
      tagGameServer* pGameServer = (tagGameServer*) pData;

      // 更新数据
      for (WORD i = 0; i < wItemCount; i++) {
        m_ServerListManager.InsertGameServer(pGameServer++);
      }

      return true;
    }
    case SUB_CS_S_SERVER_MODIFY: // 房间修改
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_CS_S_ServerModify));
      if (wDataSize != sizeof(CMD_CS_S_ServerModify))
        return false;

      // 变量定义
      CMD_CS_S_ServerModify* pServerModify = (CMD_CS_S_ServerModify*) pData;

      // 查找房间
      ASSERT(m_ServerListManager.SearchGameServer(pServerModify->wServerID));
      CGameServerItem* pGameServerItem = m_ServerListManager.SearchGameServer(pServerModify->wServerID);

      // 设置房间
      if (pGameServerItem != nullptr) {
        // 设置人数
        DWORD dwOldOnlineCount = 0, dwOldAndroidCount = 0, dwOldFullCount = 0;
        dwOldAndroidCount = pGameServerItem->m_GameServer.dwAndroidCount;
        dwOldOnlineCount = pGameServerItem->m_GameServer.dwOnLineCount;
        dwOldFullCount = pGameServerItem->m_GameServer.dwFullCount;

        // 修改房间信息
        pGameServerItem->m_GameServer.wKindID = pServerModify->wKindID;
        pGameServerItem->m_GameServer.wNodeID = pServerModify->wNodeID;
        pGameServerItem->m_GameServer.wSortID = pServerModify->wSortID;
        pGameServerItem->m_GameServer.wServerPort = pServerModify->wServerPort;
        pGameServerItem->m_GameServer.dwOnLineCount = pServerModify->dwOnLineCount;
        pGameServerItem->m_GameServer.dwFullCount = pServerModify->dwFullCount;
        pGameServerItem->m_GameServer.dwAndroidCount = pServerModify->dwAndroidCount;
        pGameServerItem->m_GameServer.dwSetPlayerCount = pServerModify->dwSetPlayerCount;
        LSTRCPYN(pGameServerItem->m_GameServer.szServerName, pServerModify->szServerName, std::size(pGameServerItem->m_GameServer.szServerName));
        LSTRCPYN(pGameServerItem->m_GameServer.szServerAddr, pServerModify->szServerAddr, std::size(pGameServerItem->m_GameServer.szServerAddr));
        LSTRCPYN(pGameServerItem->m_GameServer.szGameInfomation, pServerModify->szGameInfomation,
                 std::size(pGameServerItem->m_GameServer.szGameInfomation));

        // 目录人数
        CGameKindItem* pGameKindItem = m_ServerListManager.SearchGameKind(pGameServerItem->m_GameServer.wKindID);
        if (pGameKindItem != nullptr) {
          tagGameServer* pGameServer = &pGameServerItem->m_GameServer;
          pGameKindItem->m_GameKind.dwOnLineCount -= dwOldOnlineCount;
          pGameKindItem->m_GameKind.dwOnLineCount += pGameServer->dwOnLineCount;

          pGameKindItem->m_GameKind.dwFullCount -= dwOldFullCount;
          pGameKindItem->m_GameKind.dwFullCount += pGameServer->dwFullCount;

          pGameKindItem->m_GameKind.dwAndroidCount -= dwOldAndroidCount;
          pGameKindItem->m_GameKind.dwAndroidCount += pServerModify->dwAndroidCount;

          pGameKindItem->m_GameKind.dwSetCount = pServerModify->dwSetPlayerCount;
        }
      }

      return true;
    }
    case SUB_CS_S_SERVER_REMOVE: // 房间删除
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_CS_S_ServerRemove));
      if (wDataSize != sizeof(CMD_CS_S_ServerRemove))
        return false;

      // 变量定义
      CMD_CS_S_ServerRemove* pServerRemove = (CMD_CS_S_ServerRemove*) pData;

      // 变量定义
      m_ServerListManager.DeleteGameServer(pServerRemove->wServerID);

      DBR_GR_CLOSE_ROOM_SERVER_ID closeRoomServerID;
      closeRoomServerID.dwServerID = pServerRemove->wServerID;

      // if (m_bHasPrsnlRM)
      //{
      //	//向数据可发送查询每个房间信息的请求
      //	database_engine_->PostDataBaseRequest(DBR_GR_CLOSE_ROOM_WRITE_DISSUME_TIME,  0,  &closeRoomServerID,
      // sizeof(DBR_GR_CLOSE_ROOM_SERVER_ID));
      // }
      return true;
    }
    case SUB_CS_S_SERVER_FINISH: // 房间完成
    {
      // 清理列表
      m_ServerListManager.CleanServerItem();

      // 事件处理
      service_units->OnCorrespondResult(ER_SUCCESS);

      return true;
    }
    case SUB_CS_S_MATCH_INSERT: // 比赛插入
    {
      // 效验参数
      ASSERT(wDataSize % sizeof(tagGameMatch) == 0);
      if (wDataSize % sizeof(tagGameMatch) != 0)
        return false;

      // 变量定义
      WORD wItemCount = wDataSize / sizeof(tagGameMatch);
      tagGameMatch* pGameMatch = (tagGameMatch*) pData;

      // 更新数据
      for (WORD i = 0; i < wItemCount; i++) {
        CGameServerItem* pGameServerItem = m_ServerListManager.SearchGameServer(pGameMatch->wServerID);
        if (pGameServerItem != nullptr) {
          CopyMemory(&pGameServerItem->m_GameMatch, pGameMatch++, sizeof(pGameServerItem->m_GameMatch));
        }
      }

      return true;
    }
  }

  return true;
}

// 约战服务
bool CDispatchEngineSink::OnTCPSocketPersonalServiceInfo(WORD wSubCmdID, VOID* pData, WORD wDataSize) {
  switch (wSubCmdID) {
    case SUB_CS_S_QUERY_GAME_SERVER_RESULT: // 查询结果
    {
      // 效验数据
      ASSERT(wDataSize == sizeof(CMD_CS_S_QueryGameServerResult));
      if (wDataSize != sizeof(CMD_CS_S_QueryGameServerResult))
        return true;

      CMD_CS_S_QueryGameServerResult* pQueryGameServerResult = (CMD_CS_S_QueryGameServerResult*) pData;

      // 判断在线
      ASSERT(LOWORD(pQueryGameServerResult->dwSocketID) < init_parameter_->max_connect_);
      if ((bind_parameter_ + LOWORD(pQueryGameServerResult->dwSocketID))->dwSocketID != pQueryGameServerResult->dwSocketID)
        return true;

      // 构造数据
      CMD_MB_QueryGameServerResult QueryGameServerResult;
      ZeroMemory(&QueryGameServerResult, sizeof(CMD_MB_QueryGameServerResult));

      QueryGameServerResult.dwServerID = pQueryGameServerResult->dwServerID;
      QueryGameServerResult.bCanCreateRoom = pQueryGameServerResult->bCanCreateRoom;
      LSTRCPYN(QueryGameServerResult.szErrDescrybe, pQueryGameServerResult->szErrDescrybe, std::size(QueryGameServerResult.szErrDescrybe));

      // 发送数据
      network_engine_->SendData(pQueryGameServerResult->dwSocketID, MDM_MB_PERSONAL_SERVICE, SUB_MB_QUERY_GAME_SERVER_RESULT, &QueryGameServerResult,
                                sizeof(CMD_MB_QueryGameServerResult));

      return true;
    }
    case SUB_CS_S_SEARCH_TABLE_RESULT: {
      // 校验数据
      ASSERT(wDataSize == sizeof(CMD_CS_S_SearchTableResult));
      if (wDataSize != sizeof(CMD_CS_S_SearchTableResult))
        return true;

      CMD_CS_S_SearchTableResult* pSearTableResult = (CMD_CS_S_SearchTableResult*) pData;

      // 判断在线
      ASSERT(LOWORD(pSearTableResult->dwSocketID) < init_parameter_->max_connect_);
      if ((bind_parameter_ + LOWORD(pSearTableResult->dwSocketID))->dwSocketID != pSearTableResult->dwSocketID)
        return true;

      // 构造数据
      CMD_MB_SearchResult SearchResult;
      ZeroMemory(&SearchResult, sizeof(CMD_MB_SearchResult));

      SearchResult.dwServerID = pSearTableResult->dwServerID;
      SearchResult.dwTableID = pSearTableResult->dwTableID;
      SearchResult.dwKindID = pSearTableResult->dwKindID;

      // 发送消息
      network_engine_->SendData(pSearTableResult->dwSocketID, MDM_MB_PERSONAL_SERVICE, SUB_MB_SEARCH_RESULT, &SearchResult,
                                sizeof(CMD_MB_SearchResult));

      return true;
    }
    case SUB_CS_S_DISSUME_SEARCH_TABLE_RESULT: {
      // 校验数据
      ASSERT(wDataSize == sizeof(CMD_CS_S_SearchTableResult));
      if (wDataSize != sizeof(CMD_CS_S_SearchTableResult))
        return true;

      CMD_CS_S_SearchTableResult* pSearTableResult = (CMD_CS_S_SearchTableResult*) pData;

      // 判断在线
      ASSERT(LOWORD(pSearTableResult->dwSocketID) < init_parameter_->max_connect_);
      if ((bind_parameter_ + LOWORD(pSearTableResult->dwSocketID))->dwSocketID != pSearTableResult->dwSocketID)
        return true;

      // 构造数据
      CMD_MB_DissumeSearchResult SearchResult;
      ZeroMemory(&SearchResult, sizeof(CMD_MB_DissumeSearchResult));

      SearchResult.dwServerID = pSearTableResult->dwServerID;
      SearchResult.dwTableID = pSearTableResult->dwTableID;

      // 发送消息
      // 发送消息
      network_engine_->SendData(pSearTableResult->dwSocketID, MDM_MB_PERSONAL_SERVICE, SUB_MB_DISSUME_SEARCH_RESULT, &SearchResult,
                                sizeof(CMD_MB_DissumeSearchResult));

      return true;
    }
    case SUB_CS_S_QUERY_ROOM_PASSWORD_RESULT: {
      // 校验数据
      ASSERT(wDataSize == sizeof(CMD_CS_S_QueryRoomPasswordResult));
      if (wDataSize != sizeof(CMD_CS_S_QueryRoomPasswordResult))
        return true;

      CMD_CS_S_QueryRoomPasswordResult* pSearTableResult = (CMD_CS_S_QueryRoomPasswordResult*) pData;

      // 判断在线
      ASSERT(LOWORD(pSearTableResult->dwSocketID) < init_parameter_->max_connect_);
      if ((bind_parameter_ + LOWORD(pSearTableResult->dwSocketID))->dwSocketID != pSearTableResult->dwSocketID)
        return true;

      // 构造数据
      CMD_MB_QueryRoomPwdResult SearchResult;
      ZeroMemory(&SearchResult, sizeof(CMD_MB_QueryRoomPwdResult));

      SearchResult.dwRoomDwd = pSearTableResult->dwRoomPassword;

      // 发送消息
      // 发送消息
      network_engine_->SendData(pSearTableResult->dwSocketID, MDM_MB_PERSONAL_SERVICE, SUB_MB_QUERY_ROOM_PASSWORD_RESULT, &SearchResult,
                                sizeof(CMD_MB_DissumeSearchResult));

      return true;
    }
    case SUB_CS_C_QUERY_PERSONAL_ROOM_LIST_RESULT: {
      // 校验数据
      ASSERT(wDataSize == sizeof(CMD_CS_C_HostCreatRoomInfo));
      if (wDataSize != sizeof(CMD_CS_C_HostCreatRoomInfo))
        return true;

      CMD_CS_C_HostCreatRoomInfo* pHostCreatRoomInfo = (CMD_CS_C_HostCreatRoomInfo*) pData;

      // 向数据可发送查询每个房间信息的请求
      database_engine_->PostDataBaseRequest(DBR_MB_QUERY_PERSONAL_ROOM_INFO, pHostCreatRoomInfo->wSocketID, pHostCreatRoomInfo,
                                            sizeof(CMD_CS_C_HostCreatRoomInfo));

      return true;
    }
  }

  return true;
}

// 远程服务
bool CDispatchEngineSink::OnTCPSocketMainRemoteService(WORD wSubCmdID, VOID* pData, WORD wDataSize) {
  switch (wSubCmdID) {
    case SUB_CS_S_SEARCH_CORRESPOND: // 协调查找
    {
      // 变量定义
      CMD_CS_S_SearchCorrespond* pSearchCorrespond = (CMD_CS_S_SearchCorrespond*) pData;

      // 效验参数
      ASSERT(wDataSize <= sizeof(CMD_CS_S_SearchCorrespond));
      ASSERT(wDataSize >= (sizeof(CMD_CS_S_SearchCorrespond) - sizeof(pSearchCorrespond->UserRemoteInfo)));
      ASSERT(wDataSize == (sizeof(CMD_CS_S_SearchCorrespond) - sizeof(pSearchCorrespond->UserRemoteInfo) +
                           pSearchCorrespond->wUserCount * sizeof(pSearchCorrespond->UserRemoteInfo[0])));

      // 效验参数
      if (wDataSize > sizeof(CMD_CS_S_SearchCorrespond))
        return false;
      if (wDataSize < (sizeof(CMD_CS_S_SearchCorrespond) - sizeof(pSearchCorrespond->UserRemoteInfo)))
        return false;
      if (wDataSize != (sizeof(CMD_CS_S_SearchCorrespond) - sizeof(pSearchCorrespond->UserRemoteInfo) +
                        pSearchCorrespond->wUserCount * sizeof(pSearchCorrespond->UserRemoteInfo[0])))
        return false;

      // 判断在线
      ASSERT(LOWORD(pSearchCorrespond->dwSocketID) < init_parameter_->max_connect_);
      if ((bind_parameter_ + LOWORD(pSearchCorrespond->dwSocketID))->dwSocketID != pSearchCorrespond->dwSocketID)
        return true;

      // 变量定义
      CMD_GP_S_SearchCorrespond SearchCorrespond;
      ZeroMemory(&SearchCorrespond, sizeof(SearchCorrespond));

      // 设置变量
      for (WORD i = 0; i < pSearchCorrespond->wUserCount; i++) {
        // 数据效验
        ASSERT(SearchCorrespond.wUserCount < std::size(SearchCorrespond.UserRemoteInfo));
        if (SearchCorrespond.wUserCount >= std::size(SearchCorrespond.UserRemoteInfo))
          break;

        // 设置变量
        WORD wIndex = SearchCorrespond.wUserCount++;
        CopyMemory(&SearchCorrespond.UserRemoteInfo[wIndex], &pSearchCorrespond->UserRemoteInfo[i], sizeof(SearchCorrespond.UserRemoteInfo[wIndex]));
      }

      // 发送数据
      WORD wHeadSize = sizeof(SearchCorrespond) - sizeof(SearchCorrespond.UserRemoteInfo);
      WORD wItemSize = sizeof(SearchCorrespond.UserRemoteInfo[0]) * SearchCorrespond.wUserCount;
      network_engine_->SendData(pSearchCorrespond->dwSocketID, MDM_GP_REMOTE_SERVICE, SUB_GP_S_SEARCH_CORRESPOND, &SearchCorrespond,
                                wHeadSize + wItemSize);

      return true;
    }
    case SUB_CS_S_SEARCH_ALLCORRESPOND: {
      // 变量定义
      CMD_CS_S_SearchAllCorrespond* pSearchAllCorrespond = (CMD_CS_S_SearchAllCorrespond*) pData;

      BYTE cbDataBuffer[SOCKET_TCP_PACKET] = {0};
      WORD cbDataSize = sizeof(CMD_GP_S_SearchAllCorrespond) - sizeof(tagUserRemoteInfo);
      CMD_GP_S_SearchAllCorrespond* pSearchCorrespond = (CMD_GP_S_SearchAllCorrespond*) cbDataBuffer;
      pSearchCorrespond->dwCount = pSearchAllCorrespond->dwCount;
      int nCount = (int) (pSearchAllCorrespond->dwCount);
      for (int i = 0; i < nCount; i++) {
        memcpy(&pSearchCorrespond->UserRemoteInfo[i], &pSearchAllCorrespond->UserRemoteInfo[i], sizeof(tagUserRemoteInfo));

        cbDataSize += sizeof(tagUserRemoteInfo);
      }

      if (pSearchAllCorrespond->dwCount == 0)
        cbDataSize = sizeof(CMD_GP_S_SearchAllCorrespond);
      network_engine_->SendData(pSearchAllCorrespond->dwSocketID, MDM_GP_REMOTE_SERVICE, SUB_GP_S_SEARCH_ALLCORRESPOND, pSearchCorrespond,
                                cbDataSize);
      return true;
    }
  }

  return true;
}

// 管理服务
bool CDispatchEngineSink::OnTCPSocketMainManagerService(WORD wSubCmdID, VOID* pData, WORD wDataSize) {
  switch (wSubCmdID) {
    case SUB_CS_S_PLATFORM_PARAMETER: // 平台参数
    {
      // 清除数据
      m_wTaskCountBuffer = 0;
      ZeroMemory(m_TaskParameterBuffer, sizeof(m_TaskParameterBuffer));

      // 平台参数
      // database_engine_->PostDataBaseRequest(DBR_GP_LOAD_PLATFORM_PARAMETER,0,nullptr,0);

      // 加载任务
      // database_engine_->PostDataBaseRequest(DBR_GP_LOAD_TASK_LIST, 0, nullptr, 0);

      // 会员参数
      // database_engine_->PostDataBaseRequest(DBR_GP_LOAD_MEMBER_PARAMETER,0,nullptr,0);

      // 签到奖励
      // database_engine_->PostDataBaseRequest(DBR_GP_LOAD_CHECKIN_REWARD, 0, nullptr, 0);

      // 加载低保
      // database_engine_->PostDataBaseRequest(DBR_GP_LOAD_BASEENSURE, 0, nullptr, 0);

      // 加载认证
      // database_engine_->PostDataBaseRequest(DBR_GP_LOAD_REAL_AUTH, 0, nullptr, 0);

      // 道具配置
      m_GamePropertyListManager.DisusePropertyTypeItem();
      m_GamePropertyListManager.DisusePropertyItem();
      database_engine_->PostDataBaseRequest(DBR_GP_LOAD_GAME_PROPERTY_LIST, 0, nullptr, 0);

      return true;
    }
  }

  return true;
}

// 登录处理
bool CDispatchEngineSink::OnTCPNetworkMainPCLogon(WORD wSubCmdID, VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  switch (wSubCmdID) {
    case SUB_GP_LOGON_GAMEID: // I D 登录
    {
      return OnTCPNetworkSubPCLogonGameID(pData, wDataSize, dwSocketID);
    }
    case SUB_GP_LOGON_ACCOUNTS: // 帐号登录
    {
      return OnTCPNetworkSubPCLogonAccounts(pData, wDataSize, dwSocketID);
    }
    case SUB_GP_LOGON_MANAGETOOL: // 工具登录
    {
      return OnTCPNetworkSubPCLogonManageTool(pData, wDataSize, dwSocketID);
    }
    case SUB_GP_REGISTER_ACCOUNTS: // 帐号注册
    {
      return OnTCPNetworkSubPCRegisterAccounts(pData, wDataSize, dwSocketID);
    }
    case SUB_GP_VERIFY_INDIVIDUAL: // 校验资料
    {
      return OnTCPNetworkSubPCVerifyIndividual(pData, wDataSize, dwSocketID);
    }
    case SUB_GP_LOGON_VISITOR: // 游客登录
    {
      return OnTCPNetworkSubPCLogonVisitor(pData, wDataSize, dwSocketID);
    }
    case SUB_GP_QUERY_VERIFY_CODE: // 查询验证码
    {
      return OnTCPNetworkSubPCQueryVerifyCode(pData, wDataSize, dwSocketID);
    }
  }

  return false;
}

// 列表处理
bool CDispatchEngineSink::OnTCPNetworkMainPCServerList(WORD wSubCmdID, VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  switch (wSubCmdID) {
    case SUB_GP_GET_LIST: // 获取列表
    {
      // 发送列表
      SendGameTypeInfo(dwSocketID);
      SendGameKindInfo(dwSocketID);

      // 发送列表
      if (init_parameter_->delay_list_ == FALSE) {
        // 发送列表
        SendGamePageInfo(dwSocketID, INVALID_WORD);
        SendGameNodeInfo(dwSocketID, INVALID_WORD);
        // SendGameServerInfo(dwSocketID,INVALID_WORD);
        SendGameServerInfo(dwSocketID, INVALID_WORD, DEVICE_TYPE_PC);
      } else {
        // 发送页面
        SendGamePageInfo(dwSocketID, 0);
      }

      // 发送完成
      network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_FINISH);

      return true;
    }
    case SUB_GP_GET_SERVER: // 获取房间
    {
      // 效验数据
      ASSERT(wDataSize % sizeof(WORD) == 0);
      if (wDataSize % sizeof(WORD) != 0)
        return false;

      // 发送列表
      UINT nKindCount = wDataSize / sizeof(WORD);
      for (UINT i = 0; i < nKindCount; i++) {
        SendGameNodeInfo(dwSocketID, ((WORD*) pData)[i]);
        SendGamePageInfo(dwSocketID, ((WORD*) pData)[i]);
        // SendGameServerInfo(dwSocketID,((WORD *)pData)[i]);
        SendGameServerInfo(dwSocketID, ((WORD*) pData)[i], DEVICE_TYPE_PC);
      }

      // 发送完成
      network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_SERVER_FINISH, pData, wDataSize);

      return true;
    }
    case SUB_GP_GET_ONLINE: // 获取在线
    {
      // 变量定义
      CMD_GP_GetOnline* pGetOnline = (CMD_GP_GetOnline*) pData;
      WORD wHeadSize = (sizeof(CMD_GP_GetOnline) - sizeof(pGetOnline->wOnLineServerID));

      // 效验数据
      ASSERT((wDataSize >= wHeadSize) && (wDataSize == (wHeadSize + pGetOnline->wServerCount * sizeof(WORD))));
      if ((wDataSize < wHeadSize) || (wDataSize != (wHeadSize + pGetOnline->wServerCount * sizeof(WORD))))
        return false;

      // 变量定义
      CMD_GP_KindOnline KindOnline;
      CMD_GP_ServerOnline ServerOnline;
      ZeroMemory(&KindOnline, sizeof(KindOnline));
      ZeroMemory(&ServerOnline, sizeof(ServerOnline));

      // 获取类型
      CKindItemMap::iterator* KindPosition = nullptr;
      do {
        // 获取类型
        CGameKindItem* pGameKindItem = m_ServerListManager.EmunGameKindItem(KindPosition);

        // 设置变量
        if (pGameKindItem != nullptr) {
          WORD wKindIndex = KindOnline.wKindCount++;
          KindOnline.OnLineInfoKind[wKindIndex].wKindID = pGameKindItem->m_GameKind.wKindID;
          KindOnline.OnLineInfoKind[wKindIndex].dwOnLineCount = pGameKindItem->m_GameKind.dwOnLineCount + pGameKindItem->m_GameKind.dwAndroidCount;
        }

        // 溢出判断
        if (KindOnline.wKindCount >= std::size(KindOnline.OnLineInfoKind)) {
          ASSERT(FALSE);
          break;
        }

      } while (KindPosition != nullptr);

      // 获取房间
      for (WORD i = 0; i < pGetOnline->wServerCount; i++) {
        // 获取房间
        WORD wServerID = pGetOnline->wOnLineServerID[i];
        CGameServerItem* pGameServerItem = m_ServerListManager.SearchGameServer(wServerID);

        // 设置变量
        if (pGameServerItem != nullptr) {
          WORD wServerIndex = ServerOnline.wServerCount++;
          ServerOnline.OnLineInfoServer[wServerIndex].wServerID = wServerID;
          ServerOnline.OnLineInfoServer[wServerIndex].dwOnLineCount = pGameServerItem->m_GameServer.dwOnLineCount +
                                                                      pGameServerItem->m_GameServer.dwAndroidCount +
                                                                      pGameServerItem->m_GameServer.dwSetPlayerCount;
        }

        // 溢出判断
        if (ServerOnline.wServerCount >= std::size(ServerOnline.OnLineInfoServer)) {
          ASSERT(FALSE);
          break;
        }
      }

      // 类型在线
      if (KindOnline.wKindCount > 0) {
        WORD wHeadSize = sizeof(KindOnline) - sizeof(KindOnline.OnLineInfoKind);
        WORD wSendSize = wHeadSize + KindOnline.wKindCount * sizeof(KindOnline.OnLineInfoKind[0]);
        network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GR_KINE_ONLINE, &KindOnline, wSendSize);
      }

      // 房间在线
      if (ServerOnline.wServerCount > 0) {
        WORD wHeadSize = sizeof(ServerOnline) - sizeof(ServerOnline.OnLineInfoServer);
        WORD wSendSize = wHeadSize + ServerOnline.wServerCount * sizeof(ServerOnline.OnLineInfoServer[0]);
        network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GR_SERVER_ONLINE, &ServerOnline, wSendSize);
      }

      return true;
    }
    case SUB_GP_GET_COLLECTION: // 获取收藏
    {
      return true;
    }
    case SUB_GP_GET_PROPERTY: // 获取道具
    {
      // 道具配置
      SendGamePropertyTypeInfo(dwSocketID);
      SendGamePropertyRelatInfo(dwSocketID);
      SendGamePropertyInfo(dwSocketID);
      SendGamePropertySubInfo(dwSocketID);
      network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_PROPERTY_FINISH);

      return true;
    }
  }

  return false;
}

// 服务处理
bool CDispatchEngineSink::OnTCPNetworkMainPCUserService(WORD wSubCmdID, VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  switch (wSubCmdID) {
    case SUB_GP_MODIFY_MACHINE: // 绑定机器
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_ModifyMachine));
      if (wDataSize != sizeof(CMD_GP_ModifyMachine))
        return false;

      // 处理消息
      CMD_GP_ModifyMachine* pModifyMachine = (CMD_GP_ModifyMachine*) pData;
      pModifyMachine->szPassword[std::size(pModifyMachine->szPassword) - 1] = 0;

      // 变量定义
      DBR_GP_ModifyMachine ModifyMachine;
      ZeroMemory(&ModifyMachine, sizeof(ModifyMachine));

      // 构造数据
      ModifyMachine.cbBind = pModifyMachine->cbBind;
      ModifyMachine.dwUserID = pModifyMachine->dwUserID;
      ModifyMachine.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(ModifyMachine.szPassword, pModifyMachine->szPassword, std::size(ModifyMachine.szPassword));
      LSTRCPYN(ModifyMachine.szMachineID, pModifyMachine->szMachineID, std::size(ModifyMachine.szMachineID));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_MODIFY_MACHINE, dwSocketID, &ModifyMachine, sizeof(ModifyMachine));

      return true;
    }
    case SUB_GP_MODIFY_LOGON_PASS: // 修改密码
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_ModifyLogonPass));
      if (wDataSize != sizeof(CMD_GP_ModifyLogonPass))
        return false;

      // 处理消息
      CMD_GP_ModifyLogonPass* pModifyLogonPass = (CMD_GP_ModifyLogonPass*) pData;
      pModifyLogonPass->szDesPassword[std::size(pModifyLogonPass->szDesPassword) - 1] = 0;
      pModifyLogonPass->szScrPassword[std::size(pModifyLogonPass->szScrPassword) - 1] = 0;

      // 变量定义
      DBR_GP_ModifyLogonPass ModifyLogonPass;
      ZeroMemory(&ModifyLogonPass, sizeof(ModifyLogonPass));

      // 构造数据
      ModifyLogonPass.dwUserID = pModifyLogonPass->dwUserID;
      ModifyLogonPass.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(ModifyLogonPass.szDesPassword, pModifyLogonPass->szDesPassword, std::size(ModifyLogonPass.szDesPassword));
      LSTRCPYN(ModifyLogonPass.szScrPassword, pModifyLogonPass->szScrPassword, std::size(ModifyLogonPass.szScrPassword));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_MODIFY_LOGON_PASS, dwSocketID, &ModifyLogonPass, sizeof(ModifyLogonPass));

      return true;
    }
    case SUB_GP_MODIFY_INSURE_PASS: // 修改密码
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_ModifyInsurePass));
      if (wDataSize != sizeof(CMD_GP_ModifyInsurePass))
        return false;

      // 处理消息
      CMD_GP_ModifyInsurePass* pModifyInsurePass = (CMD_GP_ModifyInsurePass*) pData;
      pModifyInsurePass->szDesPassword[std::size(pModifyInsurePass->szDesPassword) - 1] = 0;
      pModifyInsurePass->szScrPassword[std::size(pModifyInsurePass->szScrPassword) - 1] = 0;

      // 变量定义
      DBR_GP_ModifyInsurePass ModifyInsurePass;
      ZeroMemory(&ModifyInsurePass, sizeof(ModifyInsurePass));

      // 构造数据
      ModifyInsurePass.dwUserID = pModifyInsurePass->dwUserID;
      ModifyInsurePass.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(ModifyInsurePass.szDesPassword, pModifyInsurePass->szDesPassword, std::size(ModifyInsurePass.szDesPassword));
      LSTRCPYN(ModifyInsurePass.szScrPassword, pModifyInsurePass->szScrPassword, std::size(ModifyInsurePass.szScrPassword));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_MODIFY_INSURE_PASS, dwSocketID, &ModifyInsurePass, sizeof(ModifyInsurePass));

      return true;
    }
    case SUB_GP_MODIFY_UNDER_WRITE: // 修改签名
    {
      // 变量定义
      CMD_GP_ModifyUnderWrite* pModifyUnderWrite = (CMD_GP_ModifyUnderWrite*) pData;

      // 效验参数
      ASSERT(wDataSize <= sizeof(CMD_GP_ModifyUnderWrite));
      ASSERT(wDataSize >= (sizeof(CMD_GP_ModifyUnderWrite) - sizeof(pModifyUnderWrite->szUnderWrite)));
      ASSERT(wDataSize ==
             (sizeof(CMD_GP_ModifyUnderWrite) - sizeof(pModifyUnderWrite->szUnderWrite) + CountStringBuffer(pModifyUnderWrite->szUnderWrite)));

      // 效验参数
      if (wDataSize > sizeof(CMD_GP_ModifyUnderWrite))
        return false;
      if (wDataSize < (sizeof(CMD_GP_ModifyUnderWrite) - sizeof(pModifyUnderWrite->szUnderWrite)))
        return false;
      if (wDataSize !=
          (sizeof(CMD_GP_ModifyUnderWrite) - sizeof(pModifyUnderWrite->szUnderWrite) + CountStringBuffer(pModifyUnderWrite->szUnderWrite)))
        return false;

      // 处理消息
      pModifyUnderWrite->szPassword[std::size(pModifyUnderWrite->szPassword) - 1] = 0;
      pModifyUnderWrite->szUnderWrite[std::size(pModifyUnderWrite->szUnderWrite) - 1] = 0;

      // 变量定义
      DBR_GP_ModifyUnderWrite ModifyUnderWrite;
      ZeroMemory(&ModifyUnderWrite, sizeof(ModifyUnderWrite));

      // 构造数据
      ModifyUnderWrite.dwUserID = pModifyUnderWrite->dwUserID;
      ModifyUnderWrite.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(ModifyUnderWrite.szPassword, pModifyUnderWrite->szPassword, std::size(ModifyUnderWrite.szPassword));
      LSTRCPYN(ModifyUnderWrite.szUnderWrite, pModifyUnderWrite->szUnderWrite, std::size(ModifyUnderWrite.szUnderWrite));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_MODIFY_UNDER_WRITE, dwSocketID, &ModifyUnderWrite, sizeof(ModifyUnderWrite));

      return true;
    }
    case SUB_GP_REAL_AUTH_QUERY: // 实名认证
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_RealAuth));

      // 变量定义
      CMD_GP_RealAuth* pCmdData = (CMD_GP_RealAuth*) pData;

      // 处理消息
      pCmdData->szPassword[std::size(pCmdData->szPassword) - 1] = 0;
      pCmdData->szCompellation[std::size(pCmdData->szCompellation) - 1] = 0;
      pCmdData->szPassPortID[std::size(pCmdData->szPassPortID) - 1] = 0;

      // 变量定义
      DBR_GP_RealAuth DataPara;
      ZeroMemory(&DataPara, sizeof(DataPara));

      // 构造数据
      DataPara.dwUserID = pCmdData->dwUserID;
      DataPara.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(DataPara.szPassword, pCmdData->szPassword, std::size(DataPara.szPassword));
      LSTRCPYN(DataPara.szCompellation, pCmdData->szCompellation, std::size(DataPara.szCompellation));
      LSTRCPYN(DataPara.szPassPortID, pCmdData->szPassPortID, std::size(DataPara.szPassPortID));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_MODIFY_REAL_AUTH, dwSocketID, &DataPara, sizeof(DataPara));

      return true;
    }
    case SUB_GP_SYSTEM_FACE_INFO: // 修改头像
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_SystemFaceInfo));
      if (wDataSize != sizeof(CMD_GP_SystemFaceInfo))
        return false;

      // 处理消息
      CMD_GP_SystemFaceInfo* pSystemFaceInfo = (CMD_GP_SystemFaceInfo*) pData;

      // 变量定义
      DBR_GP_ModifySystemFace ModifySystemFace;
      ZeroMemory(&ModifySystemFace, sizeof(ModifySystemFace));

      // 构造数据
      ModifySystemFace.wFaceID = pSystemFaceInfo->wFaceID;
      ModifySystemFace.dwUserID = pSystemFaceInfo->dwUserID;
      ModifySystemFace.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(ModifySystemFace.szPassword, pSystemFaceInfo->szPassword, std::size(ModifySystemFace.szPassword));
      LSTRCPYN(ModifySystemFace.szMachineID, pSystemFaceInfo->szMachineID, std::size(ModifySystemFace.szMachineID));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_MODIFY_SYSTEM_FACE, dwSocketID, &ModifySystemFace, sizeof(ModifySystemFace));

      return true;
    }
    case SUB_GP_CUSTOM_FACE_INFO: // 修改头像
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_CustomFaceInfo));
      if (wDataSize != sizeof(CMD_GP_CustomFaceInfo))
        return false;

      // 处理消息
      CMD_GP_CustomFaceInfo* pCustomFaceInfo = (CMD_GP_CustomFaceInfo*) pData;

      // 变量定义
      DBR_GP_ModifyCustomFace ModifyCustomFace;
      ZeroMemory(&ModifyCustomFace, sizeof(ModifyCustomFace));

      // 构造数据
      ModifyCustomFace.dwUserID = pCustomFaceInfo->dwUserID;
      ModifyCustomFace.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(ModifyCustomFace.szPassword, pCustomFaceInfo->szPassword, std::size(ModifyCustomFace.szPassword));
      LSTRCPYN(ModifyCustomFace.szMachineID, pCustomFaceInfo->szMachineID, std::size(ModifyCustomFace.szMachineID));
      CopyMemory(ModifyCustomFace.dwCustomFace, pCustomFaceInfo->dwCustomFace, sizeof(ModifyCustomFace.dwCustomFace));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_MODIFY_CUSTOM_FACE, dwSocketID, &ModifyCustomFace, sizeof(ModifyCustomFace));

      return true;
    }
    case SUB_GP_QUERY_INDIVIDUAL: // 查询信息
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_QueryIndividual));
      if (wDataSize != sizeof(CMD_GP_QueryIndividual))
        return false;

      // 处理消息
      CMD_GP_QueryIndividual* pQueryIndividual = (CMD_GP_QueryIndividual*) pData;

      // 变量定义
      DBR_GP_QueryIndividual QueryIndividual;
      ZeroMemory(&QueryIndividual, sizeof(QueryIndividual));

      // 构造数据
      QueryIndividual.dwUserID = pQueryIndividual->dwUserID;
      QueryIndividual.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(QueryIndividual.szPassword, pQueryIndividual->szPassword, std::size(QueryIndividual.szPassword));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_QUERY_INDIVIDUAL, dwSocketID, &QueryIndividual, sizeof(QueryIndividual));

      return true;
    }
    case SUB_GP_BIND_SPREADER: // 绑定推广
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_BindSpreader));
      if (wDataSize != sizeof(CMD_GP_BindSpreader))
        return false;

      // 处理消息
      CMD_GP_BindSpreader* pBindSpreader = (CMD_GP_BindSpreader*) pData;

      // 变量定义
      DBR_GP_BindSpreader BindSpreader;
      ZeroMemory(&BindSpreader, sizeof(BindSpreader));

      // 构造数据
      BindSpreader.dwUserID = pBindSpreader->dwUserID;
      BindSpreader.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      BindSpreader.dwSpreaderID = pBindSpreader->dwSpreaderID;
      LSTRCPYN(BindSpreader.szPassword, pBindSpreader->szPassword, std::size(BindSpreader.szPassword));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_BIND_SPREADER, dwSocketID, &BindSpreader, sizeof(BindSpreader));

      return true;
    }
    case SUB_GP_MODIFY_INDIVIDUAL: // 修改资料
    {
      // 效验参数
      ASSERT(wDataSize >= sizeof(CMD_GP_ModifyIndividual));
      if (wDataSize < sizeof(CMD_GP_ModifyIndividual))
        return false;

      // 处理消息
      CMD_GP_ModifyIndividual* pModifyIndividual = (CMD_GP_ModifyIndividual*) pData;
      pModifyIndividual->szPassword[std::size(pModifyIndividual->szPassword) - 1] = 0;

      // 变量定义
      DBR_GP_ModifyIndividual ModifyIndividual;
      ZeroMemory(&ModifyIndividual, sizeof(ModifyIndividual));

      // 设置变量
      ModifyIndividual.dwUserID = pModifyIndividual->dwUserID;
      ModifyIndividual.cbGender = pModifyIndividual->cbGender;
      ModifyIndividual.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(ModifyIndividual.szPassword, pModifyIndividual->szPassword, std::size(ModifyIndividual.szPassword));

      // 变量定义
      VOID* pDataBuffer = nullptr;
      tagDataDescribe DataDescribe;
      CRecvPacketHelper RecvPacket(pModifyIndividual + 1, wDataSize - sizeof(CMD_GP_ModifyIndividual));

      // 扩展信息
      while (true) {
        pDataBuffer = RecvPacket.GetData(DataDescribe);
        if (DataDescribe.wDataDescribe == DTP_NULL)
          break;
        switch (DataDescribe.wDataDescribe) {
          case DTP_GP_UI_NICKNAME: // 用户昵称
          {
            ASSERT(pDataBuffer != nullptr);
            ASSERT(DataDescribe.wDataSize <= sizeof(ModifyIndividual.szNickName));
            if (DataDescribe.wDataSize <= sizeof(ModifyIndividual.szNickName)) {
              CopyMemory(&ModifyIndividual.szNickName, pDataBuffer, DataDescribe.wDataSize);
              ModifyIndividual.szNickName[std::size(ModifyIndividual.szNickName) - 1] = 0;
            }
            break;
          }
          case DTP_GP_UI_UNDER_WRITE: // 个性签名
          {
            ASSERT(pDataBuffer != nullptr);
            ASSERT(DataDescribe.wDataSize <= sizeof(ModifyIndividual.szUnderWrite));
            if (DataDescribe.wDataSize <= sizeof(ModifyIndividual.szUnderWrite)) {
              CopyMemory(&ModifyIndividual.szUnderWrite, pDataBuffer, DataDescribe.wDataSize);
              ModifyIndividual.szUnderWrite[std::size(ModifyIndividual.szUnderWrite) - 1] = 0;
            }
            break;
          }
          case DTP_GP_UI_USER_NOTE: // 用户备注
          {
            ASSERT(pDataBuffer != nullptr);
            ASSERT(DataDescribe.wDataSize <= sizeof(ModifyIndividual.szUserNote));
            if (DataDescribe.wDataSize <= sizeof(ModifyIndividual.szUserNote)) {
              CopyMemory(&ModifyIndividual.szUserNote, pDataBuffer, DataDescribe.wDataSize);
              ModifyIndividual.szUserNote[std::size(ModifyIndividual.szUserNote) - 1] = 0;
            }
            break;
          }
          case DTP_GP_UI_COMPELLATION: // 真实名字
          {
            ASSERT(pDataBuffer != nullptr);
            ASSERT(DataDescribe.wDataSize <= sizeof(ModifyIndividual.szCompellation));
            if (DataDescribe.wDataSize <= sizeof(ModifyIndividual.szCompellation)) {
              CopyMemory(&ModifyIndividual.szCompellation, pDataBuffer, DataDescribe.wDataSize);
              ModifyIndividual.szCompellation[std::size(ModifyIndividual.szCompellation) - 1] = 0;
            }
            break;
          }
          case DTP_GP_UI_PASSPORTID: // 身份证
          {
            ASSERT(pDataBuffer != nullptr);
            ASSERT(DataDescribe.wDataSize <= sizeof(ModifyIndividual.szPassPortID));
            if (DataDescribe.wDataSize <= sizeof(ModifyIndividual.szPassPortID)) {
              CopyMemory(&ModifyIndividual.szPassPortID, pDataBuffer, DataDescribe.wDataSize);
              ModifyIndividual.szPassPortID[std::size(ModifyIndividual.szPassPortID) - 1] = 0;
            }
            break;
          }
          case DTP_GP_UI_SEAT_PHONE: // 固定电话
          {
            ASSERT(pDataBuffer != nullptr);
            ASSERT(DataDescribe.wDataSize <= sizeof(ModifyIndividual.szSeatPhone));
            if (DataDescribe.wDataSize <= sizeof(ModifyIndividual.szSeatPhone)) {
              CopyMemory(ModifyIndividual.szSeatPhone, pDataBuffer, DataDescribe.wDataSize);
              ModifyIndividual.szSeatPhone[std::size(ModifyIndividual.szSeatPhone) - 1] = 0;
            }
            break;
          }
          case DTP_GP_UI_MOBILE_PHONE: // 移动电话
          {
            ASSERT(pDataBuffer != nullptr);
            ASSERT(DataDescribe.wDataSize <= sizeof(ModifyIndividual.szMobilePhone));
            if (DataDescribe.wDataSize <= sizeof(ModifyIndividual.szMobilePhone)) {
              CopyMemory(ModifyIndividual.szMobilePhone, pDataBuffer, DataDescribe.wDataSize);
              ModifyIndividual.szMobilePhone[std::size(ModifyIndividual.szMobilePhone) - 1] = 0;
            }
            break;
          }
          case DTP_GP_UI_QQ: // Q Q 号码
          {
            ASSERT(pDataBuffer != nullptr);
            ASSERT(DataDescribe.wDataSize <= sizeof(ModifyIndividual.szQQ));
            if (DataDescribe.wDataSize <= sizeof(ModifyIndividual.szQQ)) {
              CopyMemory(ModifyIndividual.szQQ, pDataBuffer, DataDescribe.wDataSize);
              ModifyIndividual.szQQ[std::size(ModifyIndividual.szQQ) - 1] = 0;
            }
            break;
          }
          case DTP_GP_UI_EMAIL: // 电子邮件
          {
            ASSERT(pDataBuffer != nullptr);
            ASSERT(DataDescribe.wDataSize <= sizeof(ModifyIndividual.szEMail));
            if (DataDescribe.wDataSize <= sizeof(ModifyIndividual.szEMail)) {
              CopyMemory(ModifyIndividual.szEMail, pDataBuffer, DataDescribe.wDataSize);
              ModifyIndividual.szEMail[std::size(ModifyIndividual.szEMail) - 1] = 0;
            }
            break;
          }
          case DTP_GP_UI_DWELLING_PLACE: // 联系地址
          {
            ASSERT(pDataBuffer != nullptr);
            ASSERT(DataDescribe.wDataSize <= sizeof(ModifyIndividual.szDwellingPlace));
            if (DataDescribe.wDataSize <= sizeof(ModifyIndividual.szDwellingPlace)) {
              CopyMemory(ModifyIndividual.szDwellingPlace, pDataBuffer, DataDescribe.wDataSize);
              ModifyIndividual.szDwellingPlace[std::size(ModifyIndividual.szDwellingPlace) - 1] = 0;
            }
            break;
          }
          case DTP_GP_UI_SPREADER: // 推广标识
          {
            ASSERT(pDataBuffer != nullptr);
            ASSERT(DataDescribe.wDataSize <= sizeof(ModifyIndividual.szSpreader));
            if (DataDescribe.wDataSize <= sizeof(ModifyIndividual.szSpreader)) {
              CopyMemory(ModifyIndividual.szSpreader, pDataBuffer, DataDescribe.wDataSize);
              ModifyIndividual.szSpreader[std::size(ModifyIndividual.szSpreader) - 1] = 0;
            }
            break;
          }
        }
      }

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_MODIFY_INDIVIDUAL, dwSocketID, &ModifyIndividual, sizeof(ModifyIndividual));

      return true;
    }
    case SUB_GP_USER_ENABLE_INSURE: // 开通银行
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_UserEnableInsure));
      if (wDataSize != sizeof(CMD_GP_UserEnableInsure))
        return false;

      // 处理消息
      CMD_GP_UserEnableInsure* pUserEnableInsure = (CMD_GP_UserEnableInsure*) pData;
      pUserEnableInsure->szLogonPass[std::size(pUserEnableInsure->szLogonPass) - 1] = 0;
      pUserEnableInsure->szInsurePass[std::size(pUserEnableInsure->szInsurePass) - 1] = 0;
      pUserEnableInsure->szMachineID[std::size(pUserEnableInsure->szMachineID) - 1] = 0;

      // 变量定义
      DBR_GP_UserEnableInsure UserEnableInsure;
      ZeroMemory(&UserEnableInsure, sizeof(UserEnableInsure));

      // 构造数据
      UserEnableInsure.dwUserID = pUserEnableInsure->dwUserID;
      UserEnableInsure.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(UserEnableInsure.szLogonPass, pUserEnableInsure->szLogonPass, std::size(UserEnableInsure.szLogonPass));
      LSTRCPYN(UserEnableInsure.szInsurePass, pUserEnableInsure->szInsurePass, std::size(UserEnableInsure.szInsurePass));
      LSTRCPYN(UserEnableInsure.szMachineID, pUserEnableInsure->szMachineID, std::size(UserEnableInsure.szMachineID));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_USER_ENABLE_INSURE, dwSocketID, &UserEnableInsure, sizeof(UserEnableInsure));

      return true;
    }
    case SUB_GP_USER_SAVE_SCORE: // 存入游戏币
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_UserSaveScore));
      if (wDataSize != sizeof(CMD_GP_UserSaveScore))
        return false;

      // 处理消息
      CMD_GP_UserSaveScore* pUserSaveScore = (CMD_GP_UserSaveScore*) pData;
      pUserSaveScore->szMachineID[std::size(pUserSaveScore->szMachineID) - 1] = 0;

      // 变量定义
      DBR_GP_UserSaveScore UserSaveScore;
      ZeroMemory(&UserSaveScore, sizeof(UserSaveScore));

      // 构造数据
      UserSaveScore.dwUserID = pUserSaveScore->dwUserID;
      UserSaveScore.lSaveScore = pUserSaveScore->lSaveScore;
      UserSaveScore.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(UserSaveScore.szMachineID, pUserSaveScore->szMachineID, std::size(UserSaveScore.szMachineID));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_USER_SAVE_SCORE, dwSocketID, &UserSaveScore, sizeof(UserSaveScore));

      return true;
    }
    case SUB_GP_USER_TAKE_SCORE: // 取出游戏币
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_UserTakeScore));
      if (wDataSize != sizeof(CMD_GP_UserTakeScore))
        return false;

      // 处理消息
      CMD_GP_UserTakeScore* pUserTakeScore = (CMD_GP_UserTakeScore*) pData;
      pUserTakeScore->szPassword[std::size(pUserTakeScore->szPassword) - 1] = 0;
      pUserTakeScore->szMachineID[std::size(pUserTakeScore->szMachineID) - 1] = 0;

      // 变量定义
      DBR_GP_UserTakeScore UserTakeScore;
      ZeroMemory(&UserTakeScore, sizeof(UserTakeScore));

      // 构造数据
      UserTakeScore.dwUserID = pUserTakeScore->dwUserID;
      UserTakeScore.lTakeScore = pUserTakeScore->lTakeScore;
      UserTakeScore.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(UserTakeScore.szPassword, pUserTakeScore->szPassword, std::size(UserTakeScore.szPassword));
      LSTRCPYN(UserTakeScore.szMachineID, pUserTakeScore->szMachineID, std::size(UserTakeScore.szMachineID));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_USER_TAKE_SCORE, dwSocketID, &UserTakeScore, sizeof(UserTakeScore));

      return true;
    }
    case SUB_GP_USER_TRANSFER_SCORE: // 转帐游戏币
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_UserTransferScore));
      if (wDataSize != sizeof(CMD_GP_UserTransferScore))
        return false;

      // 处理消息
      CMD_GP_UserTransferScore* pUserTransferScore = (CMD_GP_UserTransferScore*) pData;
      if (pUserTransferScore->dwGameID == 0 || pUserTransferScore->dwUserID == 0)
        return false;

      pUserTransferScore->szPassword[std::size(pUserTransferScore->szPassword) - 1] = 0;
      pUserTransferScore->szMachineID[std::size(pUserTransferScore->szMachineID) - 1] = 0;

      // 变量定义
      DBR_GP_UserTransferScore UserTransferScore;
      ZeroMemory(&UserTransferScore, sizeof(UserTransferScore));

      // 构造数据
      UserTransferScore.dwUserID = pUserTransferScore->dwUserID;
      UserTransferScore.lTransferScore = pUserTransferScore->lTransferScore;
      UserTransferScore.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      UserTransferScore.dwGameID = pUserTransferScore->dwGameID;
      LSTRCPYN(UserTransferScore.szPassword, pUserTransferScore->szPassword, std::size(UserTransferScore.szPassword));
      LSTRCPYN(UserTransferScore.szMachineID, pUserTransferScore->szMachineID, std::size(UserTransferScore.szMachineID));
      LSTRCPYN(UserTransferScore.szTransRemark, pUserTransferScore->szTransRemark, std::size(UserTransferScore.szTransRemark));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_USER_TRANSFER_SCORE, dwSocketID, &UserTransferScore, sizeof(UserTransferScore));

      return true;
    }
    case SUB_GP_QUERY_INSURE_INFO: // 查询银行
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_QueryInsureInfo));
      if (wDataSize != sizeof(CMD_GP_QueryInsureInfo))
        return false;

      // 处理消息
      CMD_GP_QueryInsureInfo* pQueryInsureInfo = (CMD_GP_QueryInsureInfo*) pData;

      // 变量定义
      DBR_GP_QueryInsureInfo QueryInsureInfo;
      ZeroMemory(&QueryInsureInfo, sizeof(QueryInsureInfo));

      // 构造数据
      QueryInsureInfo.dwUserID = pQueryInsureInfo->dwUserID;
      QueryInsureInfo.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(QueryInsureInfo.szPassword, pQueryInsureInfo->szPassword, std::size(QueryInsureInfo.szPassword));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_QUERY_INSURE_INFO, dwSocketID, &QueryInsureInfo, sizeof(QueryInsureInfo));

      return true;
    }
    case SUB_GP_QUERY_USER_INFO_REQUEST: // 查询用户
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_QueryUserInfoRequest));
      if (wDataSize != sizeof(CMD_GP_QueryUserInfoRequest))
        return false;

      // 处理消息
      CMD_GP_QueryUserInfoRequest* pQueryUserInfoRequest = (CMD_GP_QueryUserInfoRequest*) pData;

      // ID判断
      if (pQueryUserInfoRequest->cbByNickName == FALSE) {
        // 长度判断
        int nLen = StrLenT(pQueryUserInfoRequest->szAccounts);
        if (nLen >= 8) {
          SendInsureFailure(dwSocketID, 0, TEXT("请输入合法的玩家ID！"));
          return true;
        }

        // 合法判断
        for (int i = 0; i < nLen; i++) {
          if (pQueryUserInfoRequest->szAccounts[i] < TEXT('0') || pQueryUserInfoRequest->szAccounts[i] > TEXT('9')) {
            SendInsureFailure(dwSocketID, 0, TEXT("请输入合法的玩家ID！"));
            return true;
          }
        }
      }

      // 变量定义
      DBR_GP_QueryInsureUserInfo QueryInsureUserInfo;
      ZeroMemory(&QueryInsureUserInfo, sizeof(QueryInsureUserInfo));

      // 构造数据
      QueryInsureUserInfo.cbByNickName = pQueryUserInfoRequest->cbByNickName;
      LSTRCPYN(QueryInsureUserInfo.szAccounts, pQueryUserInfoRequest->szAccounts, std::size(QueryInsureUserInfo.szAccounts));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_QUERY_USER_INFO, dwSocketID, &QueryInsureUserInfo, sizeof(QueryInsureUserInfo));

      return true;
    }
    case SUB_GP_QUERY_TRANSFER_REBATE: // 查询返利
    {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_QueryTransferRebate));
      if (wDataSize != sizeof(CMD_GP_QueryTransferRebate))
        return false;

      // 提取数据
      CMD_GP_QueryTransferRebate* pRebate = (CMD_GP_QueryTransferRebate*) pData;
      pRebate->szPassword[std::size(pRebate->szPassword) - 1] = 0;

      // 构造结构
      DBR_GP_QueryTransferRebate Rebate;
      Rebate.dwUserID = pRebate->dwUserID;
      LSTRCPYN(Rebate.szPassword, pRebate->szPassword, std::size(Rebate.szPassword));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_QUERY_TRANSFER_REBATE, dwSocketID, &Rebate, sizeof(Rebate));

      return true;
    }
    case SUB_GP_CHECKIN_QUERY: // 查询签到
    {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_CheckInQueryInfo));
      if (wDataSize != sizeof(CMD_GP_CheckInQueryInfo))
        return false;

      // 提取数据
      CMD_GP_CheckInQueryInfo* pCheckInQueryInfo = (CMD_GP_CheckInQueryInfo*) pData;
      pCheckInQueryInfo->szPassword[std::size(pCheckInQueryInfo->szPassword) - 1] = 0;

      // 构造结构
      DBR_GP_CheckInQueryInfo CheckInQueryInfo;
      CheckInQueryInfo.dwUserID = pCheckInQueryInfo->dwUserID;
      LSTRCPYN(CheckInQueryInfo.szPassword, pCheckInQueryInfo->szPassword, std::size(CheckInQueryInfo.szPassword));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_CHECKIN_QUERY_INFO, dwSocketID, &CheckInQueryInfo, sizeof(CheckInQueryInfo));

      return true;
    }
    case SUB_GP_CHECKIN_DONE: // 执行签到
    {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_CheckInDone));
      if (wDataSize != sizeof(CMD_GP_CheckInDone))
        return false;

      // 提取数据
      CMD_GP_CheckInDone* pCheckInDone = (CMD_GP_CheckInDone*) pData;
      pCheckInDone->szPassword[std::size(pCheckInDone->szPassword) - 1] = 0;
      pCheckInDone->szMachineID[std::size(pCheckInDone->szMachineID) - 1] = 0;

      // 构造结构
      DBR_GP_CheckInDone CheckInDone;
      CheckInDone.dwUserID = pCheckInDone->dwUserID;
      CheckInDone.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(CheckInDone.szPassword, pCheckInDone->szPassword, std::size(CheckInDone.szPassword));
      LSTRCPYN(CheckInDone.szMachineID, pCheckInDone->szMachineID, std::size(CheckInDone.szMachineID));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_CHECKIN_DONE, dwSocketID, &CheckInDone, sizeof(CheckInDone));

      return true;
    }
    case SUB_GP_TASK_LOAD: // 加载任务
    {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_TaskLoadInfo));
      if (wDataSize != sizeof(CMD_GP_TaskLoadInfo))
        return false;

      // 提取数据
      CMD_GP_TaskLoadInfo* pTaskLoadInfo = (CMD_GP_TaskLoadInfo*) pData;
      pTaskLoadInfo->szPassword[std::size(pTaskLoadInfo->szPassword) - 1] = 0;

      // 构造结构
      DBR_GP_TaskQueryInfo TaskQueryInfo;
      TaskQueryInfo.dwUserID = pTaskLoadInfo->dwUserID;
      LSTRCPYN(TaskQueryInfo.szPassword, pTaskLoadInfo->szPassword, std::size(TaskQueryInfo.szPassword));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_TASK_QUERY_INFO, dwSocketID, &TaskQueryInfo, sizeof(TaskQueryInfo));

      return true;
    }
    case SUB_GP_TASK_GIVEUP: {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_TaskGiveUp));
      if (wDataSize != sizeof(CMD_GP_TaskGiveUp))
        return false;
      CMD_GP_TaskTake* pTask = (CMD_GP_TaskTake*) pData;
      pTask->szPassword[std::size(pTask->szPassword) - 1] = 0;
      pTask->szMachineID[std::size(pTask->szMachineID) - 1] = 0;

      // 构造结构
      DBR_GP_TaskGiveUP task;
      task.dwUserID = pTask->dwUserID;
      task.wTaskID = pTask->wTaskID;
      task.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(task.szPassword, pTask->szPassword, std::size(task.szPassword));
      LSTRCPYN(task.szMachineID, pTask->szMachineID, std::size(task.szMachineID));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_TASK_GIVEUP, dwSocketID, &task, sizeof(task));

      return true;
    }
    case SUB_GP_TASK_TAKE: // 领取任务
    {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_TaskTake));
      if (wDataSize != sizeof(CMD_GP_TaskTake))
        return false;

      // 提取数据
      CMD_GP_TaskTake* pTaskTake = (CMD_GP_TaskTake*) pData;
      pTaskTake->szPassword[std::size(pTaskTake->szPassword) - 1] = 0;
      pTaskTake->szMachineID[std::size(pTaskTake->szMachineID) - 1] = 0;

      // 构造结构
      DBR_GP_TaskTake TaskTake;
      TaskTake.dwUserID = pTaskTake->dwUserID;
      TaskTake.wTaskID = pTaskTake->wTaskID;
      TaskTake.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(TaskTake.szPassword, pTaskTake->szPassword, std::size(TaskTake.szPassword));
      LSTRCPYN(TaskTake.szMachineID, pTaskTake->szMachineID, std::size(TaskTake.szMachineID));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_TASK_TAKE, dwSocketID, &TaskTake, sizeof(TaskTake));

      return true;
    }
    case SUB_GP_TASK_REWARD: // 领取奖励
    {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_TaskReward));
      if (wDataSize != sizeof(CMD_GP_TaskReward))
        return false;

      // 提取数据
      CMD_GP_TaskReward* pTaskReward = (CMD_GP_TaskReward*) pData;
      pTaskReward->szPassword[std::size(pTaskReward->szPassword) - 1] = 0;
      pTaskReward->szMachineID[std::size(pTaskReward->szMachineID) - 1] = 0;

      // 构造结构
      DBR_GP_TaskReward TaskReward;
      TaskReward.dwUserID = pTaskReward->dwUserID;
      TaskReward.wTaskID = pTaskReward->wTaskID;
      TaskReward.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(TaskReward.szPassword, pTaskReward->szPassword, std::size(TaskReward.szPassword));
      LSTRCPYN(TaskReward.szMachineID, pTaskReward->szMachineID, std::size(TaskReward.szMachineID));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_TASK_REWARD, dwSocketID, &TaskReward, sizeof(TaskReward));

      return true;
    }
    case SUB_GP_BASEENSURE_LOAD: // 加载低保
    {
      // 构造结构
      CMD_GP_BaseEnsureParamter BaseEnsureParameter;
      BaseEnsureParameter.cbTakeTimes = m_BaseEnsureParameter.cbTakeTimes;
      BaseEnsureParameter.lScoreAmount = m_BaseEnsureParameter.lScoreAmount;
      BaseEnsureParameter.lScoreCondition = m_BaseEnsureParameter.lScoreCondition;

      // 投递请求
      network_engine_->SendData(dwSocketID, MDM_GP_USER_SERVICE, SUB_GP_BASEENSURE_PARAMETER, &BaseEnsureParameter, sizeof(BaseEnsureParameter));

      return true;
    }
    case SUB_GP_BASEENSURE_TAKE: // 领取低保
    {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_BaseEnsureTake));
      if (wDataSize != sizeof(CMD_GP_BaseEnsureTake))
        return false;

      // 提取数据
      CMD_GP_BaseEnsureTake* pBaseEnsureTake = (CMD_GP_BaseEnsureTake*) pData;
      pBaseEnsureTake->szPassword[std::size(pBaseEnsureTake->szPassword) - 1] = 0;
      pBaseEnsureTake->szMachineID[std::size(pBaseEnsureTake->szMachineID) - 1] = 0;

      // 构造结构
      DBR_GP_TakeBaseEnsure TakeBaseEnsure;
      TakeBaseEnsure.dwUserID = pBaseEnsureTake->dwUserID;
      TakeBaseEnsure.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(TakeBaseEnsure.szPassword, pBaseEnsureTake->szPassword, std::size(TakeBaseEnsure.szPassword));
      LSTRCPYN(TakeBaseEnsure.szMachineID, pBaseEnsureTake->szMachineID, std::size(TakeBaseEnsure.szMachineID));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_BASEENSURE_TAKE, dwSocketID, &TakeBaseEnsure, sizeof(TakeBaseEnsure));

      return true;
    }
    case SUB_GP_SPREAD_QUERY: // 推广参数
    {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_UserSpreadQuery));
      if (wDataSize != sizeof(CMD_GP_UserSpreadQuery))
        return false;

      // 提取数据
      CMD_GP_UserSpreadQuery* pUserSpreadQuery = (CMD_GP_UserSpreadQuery*) pData;

      // 构造结构
      DBR_GP_QuerySpreadInfo QuerySpreadInfo;
      QuerySpreadInfo.dwUserID = pUserSpreadQuery->dwUserID;

      // 推广奖励
      database_engine_->PostDataBaseRequest(DBR_GP_QUERY_SPREAD_INFO, dwSocketID, &QuerySpreadInfo, sizeof(QuerySpreadInfo));

      return true;
    }
    case SUB_GP_GROWLEVEL_QUERY: // 查询等级
    {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_GrowLevelQueryInfo));
      if (wDataSize != sizeof(CMD_GP_GrowLevelQueryInfo))
        return false;

      // 提取数据
      CMD_GP_GrowLevelQueryInfo* pGrowLevelQueryInfo = (CMD_GP_GrowLevelQueryInfo*) pData;
      pGrowLevelQueryInfo->szPassword[std::size(pGrowLevelQueryInfo->szPassword) - 1] = 0;
      pGrowLevelQueryInfo->szMachineID[std::size(pGrowLevelQueryInfo->szMachineID) - 1] = 0;

      // 构造结构
      DBR_GP_GrowLevelQueryInfo GrowLevelQueryInfo;
      GrowLevelQueryInfo.dwUserID = pGrowLevelQueryInfo->dwUserID;
      GrowLevelQueryInfo.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(GrowLevelQueryInfo.szPassword, pGrowLevelQueryInfo->szPassword, std::size(GrowLevelQueryInfo.szPassword));
      LSTRCPYN(GrowLevelQueryInfo.szMachineID, pGrowLevelQueryInfo->szMachineID, std::size(GrowLevelQueryInfo.szMachineID));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_GROWLEVEL_QUERY_IFNO, dwSocketID, &GrowLevelQueryInfo, sizeof(GrowLevelQueryInfo));

      return true;
    }
    case SUB_GP_MEMBER_PARAMETER: // 加载会员
    {
      // 构造结构
      CMD_GP_MemberParameterResult MemberParameter;

      MemberParameter.wMemberCount = m_wMemberCount;
      CopyMemory(MemberParameter.MemberParameter, m_MemberParameter, sizeof(tagMemberParameterNew) * m_wMemberCount);

      // 投递请求
      network_engine_->SendData(dwSocketID, MDM_GP_USER_SERVICE, SUB_GP_MEMBER_PARAMETER_RESULT, &MemberParameter, sizeof(MemberParameter));

      return true;
    }
    case SUB_GP_MEMBER_QUERY_INFO: // 会员查询
    {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_MemberQueryInfo));
      if (wDataSize != sizeof(CMD_GP_MemberQueryInfo))
        return false;

      // 提取数据
      CMD_GP_MemberQueryInfo* pMemberInfo = (CMD_GP_MemberQueryInfo*) pData;
      pMemberInfo->szPassword[std::size(pMemberInfo->szPassword) - 1] = 0;
      pMemberInfo->szMachineID[std::size(pMemberInfo->szMachineID) - 1] = 0;

      // 构造结构
      DBR_GP_MemberQueryInfo MemberInfo;
      MemberInfo.dwUserID = pMemberInfo->dwUserID;
      MemberInfo.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(MemberInfo.szPassword, pMemberInfo->szPassword, std::size(MemberInfo.szPassword));
      LSTRCPYN(MemberInfo.szMachineID, pMemberInfo->szMachineID, std::size(MemberInfo.szMachineID));

      database_engine_->PostDataBaseRequest(DBR_GP_MEMBER_QUERY_INFO, dwSocketID, &MemberInfo, sizeof(MemberInfo));

      return true;
    }
    case SUB_GP_MEMBER_DAY_PRESENT: // 会员送金
    {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_MemberDayPresent));
      if (wDataSize != sizeof(CMD_GP_MemberDayPresent))
        return false;

      // 提取数据
      CMD_GP_MemberDayPresent* pMemberInfo = (CMD_GP_MemberDayPresent*) pData;
      pMemberInfo->szPassword[std::size(pMemberInfo->szPassword) - 1] = 0;
      pMemberInfo->szMachineID[std::size(pMemberInfo->szMachineID) - 1] = 0;

      // 构造结构
      DBR_GP_MemberDayPresent MemberInfo;
      MemberInfo.dwUserID = pMemberInfo->dwUserID;
      MemberInfo.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(MemberInfo.szPassword, pMemberInfo->szPassword, std::size(MemberInfo.szPassword));
      LSTRCPYN(MemberInfo.szMachineID, pMemberInfo->szMachineID, std::size(MemberInfo.szMachineID));

      database_engine_->PostDataBaseRequest(DBR_GP_MEMBER_DAY_PRESENT, dwSocketID, &MemberInfo, sizeof(MemberInfo));

      return true;
    }
    case SUB_GP_MEMBER_DAY_GIFT: // 会员礼包
    {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_MemberDayGift));
      if (wDataSize != sizeof(CMD_GP_MemberDayGift))
        return false;

      // 提取数据
      CMD_GP_MemberDayGift* pMemberInfo = (CMD_GP_MemberDayGift*) pData;
      pMemberInfo->szPassword[std::size(pMemberInfo->szPassword) - 1] = 0;
      pMemberInfo->szMachineID[std::size(pMemberInfo->szMachineID) - 1] = 0;

      // 构造结构
      DBR_GP_MemberDayGift MemberInfo;
      MemberInfo.dwUserID = pMemberInfo->dwUserID;
      MemberInfo.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(MemberInfo.szPassword, pMemberInfo->szPassword, std::size(MemberInfo.szPassword));
      LSTRCPYN(MemberInfo.szMachineID, pMemberInfo->szMachineID, std::size(MemberInfo.szMachineID));

      database_engine_->PostDataBaseRequest(DBR_GP_MEMBER_DAY_GIFT, dwSocketID, &MemberInfo, sizeof(MemberInfo));

      return true;
    }
    case SUB_GP_EXCHANGE_QUERY: // 兑换参数
    {
      // 构造结构
      CMD_GP_ExchangeParameter ExchangeParameter;
      ZeroMemory(&ExchangeParameter, sizeof(ExchangeParameter));

      // 设置变量
      ExchangeParameter.dwExchangeRate = m_PlatformParameter.dwExchangeRate;
      ExchangeParameter.wMemberCount = m_wMemberCount;
      ExchangeParameter.dwPresentExchangeRate = m_PlatformParameter.dwPresentExchangeRate;
      ExchangeParameter.dwRateGold = m_PlatformParameter.dwRateGold;
      CopyMemory(ExchangeParameter.MemberParameter, m_MemberParameter, sizeof(tagMemberParameter) * m_wMemberCount);

      // 计算大小
      WORD wSendDataSize = sizeof(ExchangeParameter) - sizeof(ExchangeParameter.MemberParameter);
      wSendDataSize += sizeof(tagMemberParameter) * ExchangeParameter.wMemberCount;

      // 发送数据
      network_engine_->SendData(dwSocketID, MDM_GP_USER_SERVICE, SUB_GP_EXCHANGE_PARAMETER, &ExchangeParameter, wSendDataSize);

      return true;
    }
    case SUB_GP_PURCHASE_MEMBER: // 购买会员
    {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_PurchaseMember));
      if (wDataSize != sizeof(CMD_GP_PurchaseMember))
        return false;

      // 构造结构
      CMD_GP_PurchaseMember* pPurchaseMember = (CMD_GP_PurchaseMember*) pData;
      pPurchaseMember->szMachineID[std::size(pPurchaseMember->szMachineID) - 1] = 0;

      // 构造结构
      DBR_GP_PurchaseMember PurchaseMember;
      ZeroMemory(&PurchaseMember, sizeof(PurchaseMember));

      // 设置变量
      PurchaseMember.dwUserID = pPurchaseMember->dwUserID;
      PurchaseMember.cbMemberOrder = pPurchaseMember->cbMemberOrder;
      PurchaseMember.wPurchaseTime = pPurchaseMember->wPurchaseTime;
      PurchaseMember.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(PurchaseMember.szMachineID, pPurchaseMember->szMachineID, std::size(PurchaseMember.szMachineID));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_PURCHASE_MEMBER, dwSocketID, &PurchaseMember, sizeof(PurchaseMember));

      return true;
    }
    case SUB_GP_EXCHANGE_SCORE_BYINGOT: // 兑换游戏币
    {
      // 参数校验
      // ASSERT(wDataSize==sizeof(CMD_GP_ExchangeScoreByIngot));
      // if(wDataSize!=sizeof(CMD_GP_ExchangeScoreByIngot)) return false;

      ////构造结构
      // CMD_GP_ExchangeScoreByIngot * pExchangeScore = (CMD_GP_ExchangeScoreByIngot*)pData;
      // pExchangeScore->szMachineID[std::size(pExchangeScore->szMachineID)-1]=0;

      ////构造结构
      // DBR_GP_ExchangeScoreByIngot ExchangeScore;
      // ZeroMemory(&ExchangeScore,sizeof(ExchangeScore));

      ////设置变量
      // ExchangeScore.dwUserID=pExchangeScore->dwUserID;
      // ExchangeScore.lExchangeIngot=pExchangeScore->lExchangeIngot;
      // ExchangeScore.dwClientAddr=(m_pBindParameter+LOWORD(dwSocketID))->dwClientAddr;
      // LSTRCPYN(ExchangeScore.szMachineID,pExchangeScore->szMachineID,std::size(ExchangeScore.szMachineID));

      ////投递请求
      // database_engine_->PostDataBaseRequest(DBR_GP_EXCHANGE_SCORE_INGOT,dwSocketID,&ExchangeScore,sizeof(ExchangeScore));

      return true;
    }
    case SUB_GP_EXCHANGE_SCORE_BYBEANS: // 兑换游戏币
    {
      // 参数校验
      // ASSERT(wDataSize==sizeof(CMD_GP_ExchangeScoreByBeans));
      // if(wDataSize!=sizeof(CMD_GP_ExchangeScoreByBeans)) return false;

      ////构造结构
      // CMD_GP_ExchangeScoreByBeans * pExchangeScore = (CMD_GP_ExchangeScoreByBeans*)pData;
      // pExchangeScore->szMachineID[std::size(pExchangeScore->szMachineID)-1]=0;

      ////构造结构
      // DBR_GP_ExchangeScoreByBeans ExchangeScore;
      // ZeroMemory(&ExchangeScore,sizeof(ExchangeScore));

      ////设置变量
      // ExchangeScore.dwUserID=pExchangeScore->dwUserID;
      // ExchangeScore.dExchangeBeans=pExchangeScore->dExchangeBeans;
      // ExchangeScore.dwClientAddr=(m_pBindParameter+LOWORD(dwSocketID))->dwClientAddr;
      // LSTRCPYN(ExchangeScore.szMachineID,pExchangeScore->szMachineID,std::size(ExchangeScore.szMachineID));

      ////投递请求
      // database_engine_->PostDataBaseRequest(DBR_GP_EXCHANGE_SCORE_BEANS,dwSocketID,&ExchangeScore,sizeof(ExchangeScore));

      return true;
    }
    case SUB_GP_LOTTERY_CONFIG_REQ: // 请求配置
    {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_LotteryConfigReq));
      if (wDataSize != sizeof(CMD_GP_LotteryConfigReq))
        return false;

      // 构造结构
      CMD_GP_LotteryConfigReq* pLotteryConfigReq = (CMD_GP_LotteryConfigReq*) pData;
      pLotteryConfigReq->szLogonPass[std::size(pLotteryConfigReq->szLogonPass) - 1] = 0;

      // 构造结构
      DBR_GP_LotteryConfigReq LotteryConfigReq;
      ZeroMemory(&LotteryConfigReq, sizeof(LotteryConfigReq));

      // 设置变量
      LotteryConfigReq.dwUserID = pLotteryConfigReq->dwUserID;
      LotteryConfigReq.wKindID = pLotteryConfigReq->wKindID;
      LotteryConfigReq.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(LotteryConfigReq.szLogonPass, pLotteryConfigReq->szLogonPass, std::size(LotteryConfigReq.szLogonPass));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_LOTTERY_CONFIG_REQ, dwSocketID, &LotteryConfigReq, sizeof(LotteryConfigReq));

      return true;
    }
    case SUB_GP_LOTTERY_START: // 抽奖开始
    {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_LotteryStart));
      if (wDataSize != sizeof(CMD_GP_LotteryStart))
        return false;

      // 构造结构
      CMD_GP_LotteryStart* pLotteryStart = (CMD_GP_LotteryStart*) pData;
      pLotteryStart->szLogonPass[std::size(pLotteryStart->szLogonPass) - 1] = 0;
      pLotteryStart->szMachineID[std::size(pLotteryStart->szMachineID) - 1] = 0;

      // 构造结构
      DBR_GP_LotteryStart LotteryStart;
      ZeroMemory(&LotteryStart, sizeof(LotteryStart));

      // 设置变量
      LotteryStart.dwUserID = pLotteryStart->dwUserID;
      LotteryStart.wKindID = pLotteryStart->wKindID;
      LotteryStart.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(LotteryStart.szLogonPass, pLotteryStart->szLogonPass, std::size(LotteryStart.szLogonPass));
      LSTRCPYN(LotteryStart.szMachineID, pLotteryStart->szMachineID, std::size(LotteryStart.szMachineID));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_LOTTERY_START, dwSocketID, &LotteryStart, sizeof(LotteryStart));

      return true;
    }
    case SUB_GP_QUERY_USER_GAME_DATA: // 游戏数据
    {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_QueryUserGameData));
      if (wDataSize != sizeof(CMD_GP_QueryUserGameData))
        return false;

      // 构造结构
      CMD_GP_QueryUserGameData* pQueryUserGameData = (CMD_GP_QueryUserGameData*) pData;
      pQueryUserGameData->szDynamicPass[std::size(pQueryUserGameData->szDynamicPass) - 1] = 0;

      // 构造结构
      DBR_GP_QueryUserGameData QueryUserGameData;
      ZeroMemory(&QueryUserGameData, sizeof(QueryUserGameData));

      // 设置变量
      QueryUserGameData.dwUserID = pQueryUserGameData->dwUserID;
      QueryUserGameData.wKindID = pQueryUserGameData->wKindID;
      LSTRCPYN(QueryUserGameData.szDynamicPass, pQueryUserGameData->szDynamicPass, std::size(QueryUserGameData.szDynamicPass));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_QUERY_USER_GAME_DATA, dwSocketID, &QueryUserGameData, sizeof(QueryUserGameData));

      return true;
    }
    case SUB_GP_ACCOUNT_BINDING: {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_AccountBind));
      if (wDataSize != sizeof(CMD_GP_AccountBind))
        return false;

      // 处理消息
      CMD_GP_AccountBind* pAccountBind = (CMD_GP_AccountBind*) pData;
      pAccountBind->szPassword[std::size(pAccountBind->szPassword) - 1] = 0;
      pAccountBind->szBindNewAccounts[std::size(pAccountBind->szBindNewAccounts) - 1] = 0;
      pAccountBind->szBindNewPassword[std::size(pAccountBind->szBindNewPassword) - 1] = 0;
      pAccountBind->szBindNewSpreader[std::size(pAccountBind->szBindNewSpreader) - 1] = 0;

      // 变量定义
      DBR_GP_AccountBind AccountBind;
      ZeroMemory(&AccountBind, sizeof(AccountBind));

      // 构造数据
      AccountBind.dwUserID = pAccountBind->dwUserID;
      AccountBind.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      AccountBind.cbDeviceType = pAccountBind->cbDeviceType;

      LSTRCPYN(AccountBind.szPassword, pAccountBind->szPassword, std::size(AccountBind.szPassword));
      LSTRCPYN(AccountBind.szMachineID, pAccountBind->szMachineID, std::size(AccountBind.szMachineID));

      LSTRCPYN(AccountBind.szBindNewAccounts, pAccountBind->szBindNewAccounts, std::size(AccountBind.szBindNewAccounts));
      LSTRCPYN(AccountBind.szBindNewPassword, pAccountBind->szBindNewPassword, std::size(AccountBind.szBindNewPassword));
      LSTRCPYN(AccountBind.szBindNewSpreader, pAccountBind->szBindNewSpreader, std::size(AccountBind.szBindNewSpreader));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_ACCOUNT_BIND, dwSocketID, &AccountBind, sizeof(AccountBind));

      return true;
    }
    case SUB_GP_ACCOUNT_BINDING_EXISTS: {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_AccountBind_Exists));
      if (wDataSize != sizeof(CMD_GP_AccountBind_Exists))
        return false;

      // 处理消息
      CMD_GP_AccountBind_Exists* pAccountBind = (CMD_GP_AccountBind_Exists*) pData;
      pAccountBind->szPassword[std::size(pAccountBind->szPassword) - 1] = 0;
      pAccountBind->szBindExistsAccounts[std::size(pAccountBind->szBindExistsAccounts) - 1] = 0;
      pAccountBind->szBindExistsPassword[std::size(pAccountBind->szBindExistsPassword) - 1] = 0;

      // 变量定义
      DBR_GP_AccountBind_Exists AccountBind;
      ZeroMemory(&AccountBind, sizeof(AccountBind));

      // 构造数据
      AccountBind.dwUserID = pAccountBind->dwUserID;
      AccountBind.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(AccountBind.szPassword, pAccountBind->szPassword, std::size(AccountBind.szPassword));
      LSTRCPYN(AccountBind.szMachineID, pAccountBind->szMachineID, std::size(AccountBind.szMachineID));

      LSTRCPYN(AccountBind.szBindExistsAccounts, pAccountBind->szBindExistsAccounts, std::size(AccountBind.szBindExistsAccounts));
      LSTRCPYN(AccountBind.szBindExistsPassword, pAccountBind->szBindExistsPassword, std::size(AccountBind.szBindExistsPassword));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_ACCOUNT_BIND_EXISTS, dwSocketID, &AccountBind, sizeof(AccountBind));
      return true;
    }
    case SUB_GP_CREATE_CLUB: {
      ASSERT(sizeof(CMD_GP_Create_Club) == wDataSize);
      if (sizeof(CMD_GP_Create_Club) != wDataSize)
        return false;

      CMD_GP_Create_Club* pNetInfo = (CMD_GP_Create_Club*) pData;

      pNetInfo->clubName[std::size(pNetInfo->clubName) - 1] = 0;
      // ConvertUtf8ToGBK(pNetInfo->clubName, 32);

      // pNetInfo->clubName[std::size(pNetInfo->clubName)-1]=0;
      // pNetInfo->clubSummary[std::size(pNetInfo->clubSummary)-1]=0;
      // pNetInfo->clubInstruction[std::size(pNetInfo->clubInstruction)-1]=0;

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_CREATE_CLUB, dwSocketID, pNetInfo, sizeof(CMD_GP_Create_Club));
      return true;
    }
    case SUB_GP_QUERY_CLUB_LIST: // 请求俱乐部
    {
      ASSERT(sizeof(CMD_GP_GetClub_List) == wDataSize);
      if (sizeof(CMD_GP_GetClub_List) != wDataSize)
        return false;

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_QUERY_CLUB_LIST, dwSocketID, pData, sizeof(CMD_GP_GetClub_List));
      return true;
    }
    case SUB_GP_QUERY_CLUB_USER_LIST: // 请求俱乐部成员
    {
      ASSERT(sizeof(CMD_GP_GetClub_User_List) == wDataSize);
      if (sizeof(CMD_GP_GetClub_User_List) != wDataSize)
        return false;

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_QUERY_CLUB_USER_LIST, dwSocketID, pData, sizeof(CMD_GP_GetClub_User_List));
      return true;
    }
    case SUB_GP_QUERY_CLUB_NAME: // 请求俱乐部名字
    {
      ASSERT(sizeof(CMD_GP_Query_Club_Name) == wDataSize);
      if (sizeof(CMD_GP_Query_Club_Name) != wDataSize)
        return false;

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_QUERY_CLUB_NAME, dwSocketID, pData, sizeof(CMD_GP_Query_Club_Name));
      return true;
    }
    case SUB_GP_QUERY_JOIN_CLUB: // 申请加入俱乐部
    {
      ASSERT(sizeof(CMD_GP_Query_Join_Club) == wDataSize);
      if (sizeof(CMD_GP_Query_Join_Club) != wDataSize)
        return false;
      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_QUERY_JOIN_CLUB, dwSocketID, pData, sizeof(CMD_GP_Query_Join_Club));
      return true;
    }
    case SUB_GP_QUERY_QUIT_CLUB: // 请求退出俱乐部
    {
      ASSERT(sizeof(CMD_GP_Query_Quit_Club) == wDataSize);
      if (sizeof(CMD_GP_Query_Quit_Club) != wDataSize)
        return false;

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_QUERY_QUIT_CLUB, dwSocketID, pData, sizeof(CMD_GP_Query_Quit_Club));
      return true;
    }
    case SUB_GP_QUERY_CLUB_JOINUSER_LIST: {
      ASSERT(sizeof(CMD_GP_Query_Club_JoinUser_List) == wDataSize);
      if (sizeof(CMD_GP_Query_Club_JoinUser_List) != wDataSize)
        return false;

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_QUERY_CLUB_JOINUSER_LIST, dwSocketID, pData, sizeof(CMD_GP_Query_Club_JoinUser_List));
      return true;
    }
    case SUB_GP_DEAL_USER_JOIN_CLUB: {
      ASSERT(sizeof(CMD_GP_DealUserJoinClub) == wDataSize);
      if (sizeof(CMD_GP_DealUserJoinClub) != wDataSize)
        return false;

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_DEAL_USER_JOIN_CLUB, dwSocketID, pData, sizeof(CMD_GP_DealUserJoinClub));
      return true;
    }
    case SUB_GP_MOVE_JEWEL_TO_CLUB: {
      ASSERT(sizeof(CMD_GR_MoveJewelToClub) == wDataSize);
      if (sizeof(CMD_GR_MoveJewelToClub) != wDataSize)
        return false;

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_MOVE_JEWEL_TO_CLUB, dwSocketID, pData, sizeof(CMD_GR_MoveJewelToClub));
      return true;
    }
    case SUB_GP_DELETE_CLUB_USER: {
      ASSERT(sizeof(CMD_GR_DeleteClubUser) == wDataSize);
      if (sizeof(CMD_GR_DeleteClubUser) != wDataSize)
        return false;

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_DELETE_CLUB_USER, dwSocketID, pData, sizeof(CMD_GR_DeleteClubUser));
      return true;
    }

    case SUB_GP_CREATE_CLUB_ROOM_RULE: {
      ASSERT(sizeof(CMD_GR_CreateClubRoomRule) == wDataSize);
      if (sizeof(CMD_GR_CreateClubRoomRule) != wDataSize)
        return false;

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_CREATE_CLUB_ROOM_RULE, dwSocketID, pData, sizeof(CMD_GR_CreateClubRoomRule));
      return true;
    }
    case SUB_GP_GET_CLUB_ALL_ROOM_RULE: {
      ASSERT(sizeof(CMD_GR_GetClubAllRoomRule) == wDataSize);
      if (sizeof(CMD_GR_GetClubAllRoomRule) != wDataSize)
        return false;

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_GET_CLUB_ALL_ROOM_RULE, dwSocketID, pData, sizeof(CMD_GR_GetClubAllRoomRule));
      return true;
    }
    case SUB_GP_DELETE_CLUB_ROOM_RULE: {
      ASSERT(sizeof(CMD_GR_DeleteClubRoomRule) == wDataSize);
      if (sizeof(CMD_GR_DeleteClubRoomRule) != wDataSize)
        return false;

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_DELETE_CLUB_ROOM_RULE, dwSocketID, pData, sizeof(CMD_GR_DeleteClubRoomRule));
      return true;
    }
    case SUB_GP_QUERY_CLUB_ROOM_LIST: {
      ASSERT(sizeof(CMD_GP_GetClub_Room_List) == wDataSize);
      if (sizeof(CMD_GP_GetClub_Room_List) != wDataSize)
        return false;

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_GET_CLUB_ROOM_LIST, dwSocketID, pData, sizeof(CMD_GP_GetClub_Room_List));
      return true;
    }
    case SUB_GP_MODIFY_CLUB_SUMMARY: {
      ASSERT(sizeof(CMD_GR_ModifyClubSummary) == wDataSize);
      if (sizeof(CMD_GR_ModifyClubSummary) != wDataSize)
        return false;

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_MODIFY_CLUB_SUMMARY, dwSocketID, pData, sizeof(CMD_GR_ModifyClubSummary));
      return true;
    }
    case SUB_GP_JOIN_USER_TO_CLUB: {
      ASSERT(sizeof(CMD_GR_JoinUserToClub) == wDataSize);
      if (sizeof(CMD_GR_JoinUserToClub) != wDataSize)
        return false;

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_JOIN_USER_TO_CLUB, dwSocketID, pData, sizeof(CMD_GR_JoinUserToClub));
      return true;
    }
  }

  return false;
}

// 远程处理
bool CDispatchEngineSink::OnTCPNetworkMainPCRemoteService(WORD wSubCmdID, VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  switch (wSubCmdID) {
    case SUB_GP_C_SEARCH_CORRESPOND: // 协调查找
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_C_SearchCorrespond));
      if (wDataSize != sizeof(CMD_GP_C_SearchCorrespond))
        return false;

      // 处理消息
      CMD_GP_C_SearchCorrespond* pSearchCorrespond = (CMD_GP_C_SearchCorrespond*) pData;
      pSearchCorrespond->szNickName[std::size(pSearchCorrespond->szNickName) - 1] = 0;

      // 变量定义
      CMD_CS_C_SearchCorrespond SearchCorrespond;
      ZeroMemory(&SearchCorrespond, sizeof(SearchCorrespond));

      // 连接变量
      SearchCorrespond.dwSocketID = dwSocketID;
      SearchCorrespond.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;

      // 查找变量
      SearchCorrespond.dwGameID = pSearchCorrespond->dwGameID;
      LSTRCPYN(SearchCorrespond.szNickName, pSearchCorrespond->szNickName, std::size(SearchCorrespond.szNickName));

      // 发送数据
      correspond_service_->SendData(MDM_CS_REMOTE_SERVICE, SUB_CS_C_SEARCH_CORRESPOND, &SearchCorrespond, sizeof(SearchCorrespond));

      return true;
    }
    case SUB_GP_C_SEARCH_ALLCORRESPOND: {
      // 发送数据
      if (wDataSize >= sizeof(CMD_GP_C_SearchAllCorrespond)) {
        CMD_GP_C_SearchAllCorrespond* pSearchAllCorrespond = (CMD_GP_C_SearchAllCorrespond*) pData;

        DWORD cBuffer[512] = {0};
        CMD_CS_C_AllSearchCorrespond* pSearchCorrespond = (CMD_CS_C_AllSearchCorrespond*) cBuffer;

        // 连接变量
        pSearchCorrespond->dwSocketID = dwSocketID;
        pSearchCorrespond->dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;

        pSearchCorrespond->dwCount = pSearchAllCorrespond->dwCount;
        memcpy(pSearchCorrespond->dwGameID, pSearchAllCorrespond->dwGameID, sizeof(DWORD) * pSearchAllCorrespond->dwCount);

        correspond_service_->SendData(MDM_CS_REMOTE_SERVICE, SUB_CS_C_SEARCH_ALLCORRESPOND, pSearchCorrespond,
                                      sizeof(CMD_CS_C_AllSearchCorrespond) + sizeof(DWORD) * (WORD) (pSearchAllCorrespond->dwCount - 1));
      }
      return true;
    }
  }

  return false;
}

// 机器服务
bool CDispatchEngineSink::OnTCPNetworkMainAndroidService(WORD wSubCmdID, VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  switch (wSubCmdID) {
    case SUB_GP_GET_PARAMETER: // 获取参数
    {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_GetParameter));
      if (wDataSize != sizeof(CMD_GP_GetParameter))
        return false;

      // 提取数据
      CMD_GP_GetParameter* pGetParameter = (CMD_GP_GetParameter*) pData;
      ASSERT(pGetParameter != nullptr);

      // 构造结构
      DBR_GP_GetParameter GetParameter;
      GetParameter.wServerID = pGetParameter->wServerID;

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_GET_PARAMETER, dwSocketID, &GetParameter, sizeof(GetParameter));

      return true;
    }
    case SUB_GP_ADD_PARAMETER: // 添加参数
    {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_AddParameter));
      if (wDataSize != sizeof(CMD_GP_AddParameter))
        return false;

      // 提取数据
      CMD_GP_AddParameter* pAddParameter = (CMD_GP_AddParameter*) pData;
      ASSERT(pAddParameter != nullptr);

      // 构造结构
      DBR_GP_AddParameter AddParameter;
      AddParameter.wServerID = pAddParameter->wServerID;
      CopyMemory(&AddParameter.AndroidParameter, &pAddParameter->AndroidParameter, sizeof(tagAndroidParameter));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_ADD_PARAMETER, dwSocketID, &AddParameter, sizeof(AddParameter));

      return true;
    }
    case SUB_GP_MODIFY_PARAMETER: // 修改参数
    {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_ModifyParameter));
      if (wDataSize != sizeof(CMD_GP_ModifyParameter))
        return false;

      // 提取数据
      CMD_GP_ModifyParameter* pModifyParameter = (CMD_GP_ModifyParameter*) pData;
      ASSERT(pModifyParameter != nullptr);

      // 构造结构
      DBR_GP_ModifyParameter ModifyParameter;
      ModifyParameter.wServerID = pModifyParameter->wServerID;
      CopyMemory(&ModifyParameter.AndroidParameter, &pModifyParameter->AndroidParameter, sizeof(tagAndroidParameter));

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_MODIFY_PARAMETER, dwSocketID, &ModifyParameter, sizeof(ModifyParameter));

      return true;
    }
    case SUB_GP_DELETE_PARAMETER: // 删除参数
    {
      // 参数校验
      ASSERT(wDataSize == sizeof(CMD_GP_DeleteParameter));
      if (wDataSize != sizeof(CMD_GP_DeleteParameter))
        return false;

      // 提取数据
      CMD_GP_DeleteParameter* pDeleteParameter = (CMD_GP_DeleteParameter*) pData;
      ASSERT(pDeleteParameter != nullptr);

      // 构造结构
      DBR_GP_DeleteParameter DeleteParameter;
      DeleteParameter.wServerID = pDeleteParameter->wServerID;
      DeleteParameter.dwBatchID = pDeleteParameter->dwBatchID;

      // 投递请求
      database_engine_->PostDataBaseRequest(DBR_GP_DELETE_PARAMETER, dwSocketID, &DeleteParameter, sizeof(DeleteParameter));

      return true;
    }
  }

  return false;
}

bool CDispatchEngineSink::OnTCPNetworkMainPCProperty(WORD wSubCmdID, VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  switch (wSubCmdID) {
    case SUB_GP_QUERY_PROPERTY: // 加载道具
    {
      // 发送消息
      network_engine_->SendData(dwSocketID, MDM_GP_PROPERTY, SUB_GP_QUERY_PROPERTY_RESULT_FINISH);
      return true;
    }
    case SUB_GP_PROPERTY_BUY: // 道具购买
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_PropertyBuy));
      if (wDataSize != sizeof(CMD_GP_PropertyBuy))
        return false;

      // 变量定义
      CMD_GP_PropertyBuy* pPropertyBuy = (CMD_GP_PropertyBuy*) pData;

      // 变量定义
      DBR_GP_PropertyBuy PropertyRequest;
      ZeroMemory(&PropertyRequest, sizeof(PropertyRequest));

      // 购买信息
      PropertyRequest.dwUserID = pPropertyBuy->dwUserID;
      PropertyRequest.dwPropertyID = pPropertyBuy->dwPropertyID;
      PropertyRequest.dwDiamondCount = pPropertyBuy->dwDiamondCount;

      // 系统信息
      PropertyRequest.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
      LSTRCPYN(PropertyRequest.szPassword, pPropertyBuy->szPassword, std::size(PropertyRequest.szPassword));
      LSTRCPYN(PropertyRequest.szMachineID, pPropertyBuy->szMachineID, std::size(PropertyRequest.szMachineID));

      // 投递数据
      database_engine_->PostDataBaseRequest(DBR_GP_PROPERTY_BUY, dwSocketID, &PropertyRequest, sizeof(PropertyRequest));

      return true;
    }
    case SUB_GP_PROPERTY_USE: // 道具使用
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_C_PropertyUse));
      if (wDataSize != sizeof(CMD_GP_C_PropertyUse))
        return false;

      // 变量定义
      CMD_GP_C_PropertyUse* pPropertyUse = (CMD_GP_C_PropertyUse*) pData;

      // 数据效验
      ASSERT(pPropertyUse->wPropCount > 0);
      if (pPropertyUse->wPropCount == 0)
        return false;

      // 变量定义
      DBR_GP_PropertyUse PropertyUseRequest;
      ZeroMemory(&PropertyUseRequest, sizeof(PropertyUseRequest));
      PropertyUseRequest.dwUserID = pPropertyUse->dwUserID;
      PropertyUseRequest.dwRecvUserID = pPropertyUse->dwRecvUserID;
      PropertyUseRequest.wPropCount = pPropertyUse->wPropCount;
      PropertyUseRequest.dwPropID = pPropertyUse->dwPropID;
      PropertyUseRequest.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr; // 系统信息

      // 投递数据
      database_engine_->PostDataBaseRequest(DBR_GP_PROPERTY_USE, dwSocketID, &PropertyUseRequest, sizeof(PropertyUseRequest));

      return true;
    }
    case SUB_GP_QUERY_SINGLE: {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_PropertyQuerySingle));
      if (wDataSize != sizeof(CMD_GP_PropertyQuerySingle))
        return false;

      CMD_GP_PropertyQuerySingle* pQuerySingle = (CMD_GP_PropertyQuerySingle*) pData;

      DBR_GP_PropertyQuerySingle QuerySingle;
      QuerySingle.dwUserID = pQuerySingle->dwUserID;
      QuerySingle.dwPropertyID = pQuerySingle->dwPropertyID;
      LSTRCPYN(QuerySingle.szPassword, pQuerySingle->szPassword, std::size(QuerySingle.szPassword));

      // 投递数据
      database_engine_->PostDataBaseRequest(DBR_GP_QUERY_SINGLE, dwSocketID, &QuerySingle, sizeof(QuerySingle));

      return true;
    }
    case SUB_GP_QUERY_BACKPACKET: // 查询背包
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_C_BackpackProperty));
      if (wDataSize != sizeof(CMD_GP_C_BackpackProperty))
        return false;

      CMD_GP_C_BackpackProperty* pBackpackProperty = (CMD_GP_C_BackpackProperty*) pData;

      DBR_GP_QueryBackpack QueryBackpack;
      QueryBackpack.dwUserID = pBackpackProperty->dwUserID;
      QueryBackpack.dwKindID = 0;
      QueryBackpack.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;

      // 投递数据
      database_engine_->PostDataBaseRequest(DBR_GP_QUERY_BACKPACK, dwSocketID, &QueryBackpack, sizeof(QueryBackpack));
      return true;
    }
    case SUB_GP_PROPERTY_BUFF: // 道具Buff
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_C_UserPropertyBuff));
      if (wDataSize != sizeof(CMD_GP_C_UserPropertyBuff))
        return false;

      // 变量定义
      CMD_GP_C_UserPropertyBuff* pPropertyBuff = (CMD_GP_C_UserPropertyBuff*) pData;

      // 变量定义
      DBR_GP_UserPropertyBuff PropertyBuffRequest;
      ZeroMemory(&PropertyBuffRequest, sizeof(PropertyBuffRequest));
      PropertyBuffRequest.dwUserID = pPropertyBuff->dwUserID;

      // 投递数据
      database_engine_->PostDataBaseRequest(DBR_GP_USER_PROPERTY_BUFF, dwSocketID, &PropertyBuffRequest, sizeof(PropertyBuffRequest));

      return true;
    }
    case SUB_GP_QUERY_SEND_PRESENT: // 查询赠送
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_QuerySendPresent));
      if (wDataSize != sizeof(CMD_GP_QuerySendPresent))
        return false;

      // 变量定义
      CMD_GP_QuerySendPresent* pQuerySendPresent = (CMD_GP_QuerySendPresent*) pData;
      DBR_GP_QuerySendPresent QuerySendPresentRequest = {0};
      QuerySendPresentRequest.dwUserID = pQuerySendPresent->dwUserID;
      QuerySendPresentRequest.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr; // 系统信息

      database_engine_->PostDataBaseRequest(DBR_GP_QUERY_SEND_PRESENT, dwSocketID, &QuerySendPresentRequest, sizeof(QuerySendPresentRequest));
      return true;
    }
    case SUB_GP_PROPERTY_PRESENT: // 道具赠送
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_C_PropertyPresent));
      if (wDataSize != sizeof(CMD_GP_C_PropertyPresent))
        return false;

      // 变量定义
      CMD_GP_C_PropertyPresent* PropertyPresent = (CMD_GP_C_PropertyPresent*) pData;
      DBR_GP_PropertyPresent PropertyPresentRequest = {0};
      PropertyPresentRequest.dwUserID = PropertyPresent->dwUserID;
      PropertyPresentRequest.dwRecvGameID = PropertyPresent->dwRecvGameID;
      PropertyPresentRequest.dwPropID = PropertyPresent->dwPropID;
      PropertyPresentRequest.wPropCount = PropertyPresent->wPropCount;
      PropertyPresentRequest.wType = PropertyPresent->wType;
      LSTRCPYN(PropertyPresentRequest.szRecvNickName, PropertyPresent->szRecvNickName, std::size(PropertyPresentRequest.szRecvNickName));
      PropertyPresentRequest.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr; // 系统信息

      database_engine_->PostDataBaseRequest(DBR_GP_PROPERTY_PRESENT, dwSocketID, &PropertyPresentRequest, sizeof(PropertyPresentRequest));
      return true;
    }
    case SUB_GP_GET_SEND_PRESENT: // 获得赠送
    {
      // 效验参数
      ASSERT(wDataSize == sizeof(CMD_GP_C_GetSendPresent));
      if (wDataSize != sizeof(CMD_GP_C_GetSendPresent))
        return false;

      // 变量定义
      CMD_GP_C_GetSendPresent* pGetSendPresent = (CMD_GP_C_GetSendPresent*) pData;
      DBR_GP_GetSendPresent GetSendPresentRequest = {0};
      GetSendPresentRequest.dwUserID = pGetSendPresent->dwUserID;
      LSTRCPYN(GetSendPresentRequest.szPassword, pGetSendPresent->szPassword, std::size(GetSendPresentRequest.szPassword));
      GetSendPresentRequest.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr; // 系统信息

      database_engine_->PostDataBaseRequest(DBR_GP_GET_SEND_PRESENT, dwSocketID, &GetSendPresentRequest, sizeof(GetSendPresentRequest));
      return true;
    }
    default:
      ASSERT(false);
      break;
  }
  return false;
}

// 登录处理
bool CDispatchEngineSink::OnTCPNetworkMainMBLogon(WORD wSubCmdID, VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  switch (wSubCmdID) {
    case SUB_MB_LOGON_GAMEID: // I D 登录
    {
      return OnTCPNetworkSubMBLogonGameID(pData, wDataSize, dwSocketID);
    }
    case SUB_MB_LOGON_ACCOUNTS: // 帐号登录
    {
      return OnTCPNetworkSubMBLogonAccounts(pData, wDataSize, dwSocketID);
    }
    case SUB_MB_REGISTER_ACCOUNTS: // 帐号注册
    {
      return OnTCPNetworkSubMBRegisterAccounts(pData, wDataSize, dwSocketID);
    }
    case SUB_MB_LOGON_OTHERPLATFORM: // 其他平台
    {
      return OnTCPNetworkSubMBLogonOtherPlatform(pData, wDataSize, dwSocketID);
    }
    case SUB_MB_LOGON_VISITOR: // 游客登录
    {
      return OnTCPNetworkSubMBLogonVisitor(pData, wDataSize, dwSocketID);
    }
  }

  return false;
}

// 列表处理
bool CDispatchEngineSink::OnTCPNetworkMainMBServerList(WORD wSubCmdID, VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  switch (wSubCmdID) {
    case SUB_MB_GET_ONLINE: {
      return OnTCPNetworkSubMBGetOnline(pData, wDataSize, dwSocketID);
    }
  }
  return false;
}

// 视频信息
bool CDispatchEngineSink::OnTCPNetworkMBVideoPlayBackInfo(WORD wSubCmdID, VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  switch (wSubCmdID) {
    case SUB_MB_QUERY_VIDEO_INFO: {
      return OnTCPNetworkSubMBQueryVideo(pData, wDataSize, dwSocketID);
    }
    case SUB_MB_QUERY_VIDEO_DETAILS: {
      return OnTCPNetworkSubMBQueryVideoDetails(pData, wDataSize, dwSocketID);
    }
    case SUB_MB_QUERY_VIDEO_DETAILS_BY_ROOMID: {
      return OnTCPNetworkSubMBQueryVideoDetailsByRoomID(pData, wDataSize, dwSocketID);
    }
    case SUB_MB_QUERY_PLAYBACK_CODE_YZ: {
      return OnTCPNetworkSubMBQueryPlayBackCodeYZ(pData, wDataSize, dwSocketID);
    }
    case SUB_MB_QUERY_PLAYBACK_CODE: {
      return OnTCPNetworkSubMBQueryPlayBackCode(pData, wDataSize, dwSocketID);
    }
  }
  return false;
}

// 视频查询
bool CDispatchEngineSink::OnTCPNetworkSubMBQueryVideo(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 效验参数
  ASSERT(wDataSize >= sizeof(CMD_MB_C_QueryVideoInfo));
  if (wDataSize < sizeof(CMD_MB_C_QueryVideoInfo))
    return false;

  // 处理消息
  CMD_MB_C_QueryVideoInfo* pQueryVideo = (CMD_MB_C_QueryVideoInfo*) pData;

  // 变量定义
  DBR_MB_QueryVideoInfo QueryVideoInfo;
  ZeroMemory(&QueryVideoInfo, sizeof(QueryVideoInfo));

  QueryVideoInfo.iQueryType = pQueryVideo->iQueryType;
  QueryVideoInfo.dwUserID = pQueryVideo->dwUserID;
  QueryVideoInfo.dwPlayBack = pQueryVideo->dwPlayBack;
  QueryVideoInfo.dwPersonalRoomID, pQueryVideo->dwPersonalRoomID;
  QueryVideoInfo.wIndexBegin = pQueryVideo->wIndexBegin;
  QueryVideoInfo.wIndexEnd = pQueryVideo->wIndexEnd;
  QueryVideoInfo.dwClubID = pQueryVideo->dwClubID;
  // 投递请求
  database_engine_->PostDataBaseRequest(DBR_MB_QUERY_VIDEO_INFO, dwSocketID, &QueryVideoInfo, sizeof(QueryVideoInfo));

  return true;
}
// 详情查询
bool CDispatchEngineSink::OnTCPNetworkSubMBQueryVideoDetails(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 效验参数
  ASSERT(wDataSize >= sizeof(CMD_MB_C_QueryVideoDetails));
  if (wDataSize < sizeof(CMD_MB_C_QueryVideoDetails))
    return false;

  // 处理消息
  CMD_MB_C_QueryVideoDetails* pQueryVideo = (CMD_MB_C_QueryVideoDetails*) pData;

  // 变量定义
  DBR_MB_QueryVideoDetails QueryVideoDetails;
  ZeroMemory(&QueryVideoDetails, sizeof(QueryVideoDetails));

  LSTRCPYN(QueryVideoDetails.szPersonalGUID, pQueryVideo->szPersonalGUID, std::size(QueryVideoDetails.szPersonalGUID));

  // 投递请求
  database_engine_->PostDataBaseRequest(DBR_MB_QUERY_VIDEO_DETAILS, dwSocketID, &QueryVideoDetails, sizeof(QueryVideoDetails));

  return true;
}

// 详情查询
bool CDispatchEngineSink::OnTCPNetworkSubMBQueryVideoDetailsByRoomID(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 效验参数
  ASSERT(wDataSize >= sizeof(CMD_MB_C_QueryVideoDetailsByRoomID));
  if (wDataSize < sizeof(CMD_MB_C_QueryVideoDetailsByRoomID))
    return false;

  // 处理消息
  CMD_MB_C_QueryVideoDetailsByRoomID* pQueryVideo = (CMD_MB_C_QueryVideoDetailsByRoomID*) pData;

  // 变量定义
  DBR_MB_QueryVideoDetailsByRoomID QueryVideoDetails;
  ZeroMemory(&QueryVideoDetails, sizeof(QueryVideoDetails));

  QueryVideoDetails.dwPersonalRoomID = pQueryVideo->dwPersonalRoomID;

  // 投递请求
  database_engine_->PostDataBaseRequest(DBR_MB_QUERY_VIDEO_DETAILS_BY_ROOMID, dwSocketID, &QueryVideoDetails, sizeof(QueryVideoDetails));

  return true;
}
// 回放码
bool CDispatchEngineSink::OnTCPNetworkSubMBQueryPlayBackCodeYZ(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 效验参数
  ASSERT(wDataSize >= sizeof(CMD_MB_C_QueryPlayBackCodeYZ));
  if (wDataSize < sizeof(CMD_MB_C_QueryPlayBackCodeYZ))
    return false;

  // 处理消息
  CMD_MB_C_QueryPlayBackCodeYZ* pQueryCode = (CMD_MB_C_QueryPlayBackCodeYZ*) pData;

  // 变量定义
  DBR_MB_QueryPlayBackCodeYZ QueryPlayBackCode;
  ZeroMemory(&QueryPlayBackCode, sizeof(QueryPlayBackCode));

  QueryPlayBackCode.dwUserID = pQueryCode->dwUserID;
  LSTRCPYN(QueryPlayBackCode.szPersonalGUID, pQueryCode->szPersonalGUID, std::size(QueryPlayBackCode.szPersonalGUID));

  // 投递请求
  database_engine_->PostDataBaseRequest(DBR_MB_QUERY_PLAYBACK_CODE_YZ, dwSocketID, &QueryPlayBackCode, sizeof(QueryPlayBackCode));

  return true;
}

// 回放码
bool CDispatchEngineSink::OnTCPNetworkSubMBQueryPlayBackCode(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 效验参数
  ASSERT(wDataSize >= sizeof(CMD_MB_C_QueryPlayBackCode));
  if (wDataSize < sizeof(CMD_MB_C_QueryPlayBackCode))
    return false;

  // 处理消息
  CMD_MB_C_QueryPlayBackCode* pQueryCode = (CMD_MB_C_QueryPlayBackCode*) pData;

  // 变量定义
  DBR_MB_QueryPlayBackCode QueryPlayBackCode;
  ZeroMemory(&QueryPlayBackCode, sizeof(QueryPlayBackCode));

  QueryPlayBackCode.dwUserID = pQueryCode->dwUserID;
  StrnCpyT(QueryPlayBackCode.szVideoNumber, pQueryCode->szVideoNumber, std::size(QueryPlayBackCode.szVideoNumber));

  // 投递请求
  database_engine_->PostDataBaseRequest(DBR_MB_QUERY_PLAYBACK_CODE, dwSocketID, &QueryPlayBackCode, sizeof(QueryPlayBackCode));

  return true;
}
// 列表处理
bool CDispatchEngineSink::OnTCPNetworkMainMBPersonalService(WORD wSubCmdID, VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  switch (wSubCmdID) {
    case SUB_MB_QUERY_GAME_SERVER: {
      return OnTCPNetworkSubMBQueryGameServer(pData, wDataSize, dwSocketID);
    }
    case SUB_MB_SEARCH_SERVER_TABLE: {
      return OnTCPNetworkSubMBSearchServerTable(pData, wDataSize, dwSocketID);
    }
    case SUB_MB_DISSUME_SEARCH_SERVER_TABLE: {
      return OnTCPNetworkSubMBDissumeSearchServerTable(pData, wDataSize, dwSocketID);
    }
    case SUB_MB_GET_PERSONAL_PARAMETER: {
      return OnTCPNetworkSubMBPersonalParameter(pData, wDataSize, dwSocketID);
    }
    case SUB_MB_QUERY_ROOM_PASSWORD: {
      return OnTCPNetworkSubMBQueryRoomPassword(pData, wDataSize, dwSocketID);
    }
    case SUB_MB_QUERY_PERSONAL_ROOM_LIST: {
      return OnTCPNetworkSubMBQueryPersonalRoomList(pData, wDataSize, dwSocketID);
    }
    case SUB_GR_USER_QUERY_ROOM_SCORE: // 请求房间成绩
    {
      return OnTCPNetworkSubQueryUserRoomScore(pData, wDataSize, dwSocketID);
    }
    case SUB_MB_QUERY_PERSONAL_ROOM_USER_INFO: // 请求玩家的游戏豆
    {
      return OnTCPNetworkSubQueryPersonalRoomUserInfo(pData, wDataSize, dwSocketID);
    }
    case SUB_MB_ROOM_CARD_EXCHANGE_TO_SCORE: // 请求玩家的游戏豆
    {
      return OnTCPNetworkSubRoomCardExchangeToScore(pData, wDataSize, dwSocketID);
    }
    case SUB_MB_PERSONAL_RULE: // 获取私人房定制配置
    {
      return OnTCPNetworkSubMBGetPersonalRoomRule(pData, wDataSize, dwSocketID);
    }
  }
  return true;
}

// I D 登录
bool CDispatchEngineSink::OnTCPNetworkSubPCLogonGameID(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 效验参数
  ASSERT(wDataSize >= sizeof(CMD_GP_LogonGameID));
  if (wDataSize < sizeof(CMD_GP_LogonGameID)) {
    if (wDataSize < sizeof(CMD_GP_LogonGameID) - sizeof(BYTE))
      return false;
  }

  // 变量定义
  WORD wBindIndex = LOWORD(dwSocketID);
  tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

  // 处理消息
  CMD_GP_LogonGameID* pLogonGameID = (CMD_GP_LogonGameID*) pData;
  pLogonGameID->szPassword[std::size(pLogonGameID->szPassword) - 1] = 0;
  pLogonGameID->szMachineID[std::size(pLogonGameID->szMachineID) - 1] = 0;

  // 设置连接
  pBindParameter->cbClientKind = CLIENT_KIND_COMPUTER;
  pBindParameter->dwPlazaVersion = pLogonGameID->dwPlazaVersion;

  // 效验版本
  if (CheckPlazaVersion(DEVICE_TYPE_PC, pLogonGameID->dwPlazaVersion, dwSocketID, ((pLogonGameID->cbValidateFlags & LOW_VER_VALIDATE_FLAGS) != 0)) ==
      false) {
    return true;
  }

  // 变量定义
  DBR_GP_LogonGameID LogonGameID;
  ZeroMemory(&LogonGameID, sizeof(LogonGameID));

  // 附加信息
  LogonGameID.pBindParameter = (bind_parameter_ + LOWORD(dwSocketID));

  // 构造数据
  LogonGameID.dwGameID = pLogonGameID->dwGameID;
  LogonGameID.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
  LSTRCPYN(LogonGameID.szPassword, pLogonGameID->szPassword, std::size(LogonGameID.szPassword));
  LSTRCPYN(LogonGameID.szMachineID, pLogonGameID->szMachineID, std::size(LogonGameID.szMachineID));
  LogonGameID.cbNeeValidateMBCard = (pLogonGameID->cbValidateFlags & MB_VALIDATE_FLAGS);

  // 投递请求
  database_engine_->PostDataBaseRequest(DBR_GP_LOGON_GAMEID, dwSocketID, &LogonGameID, sizeof(LogonGameID));

  return true;
}

// 帐号登录
bool CDispatchEngineSink::OnTCPNetworkSubPCLogonAccounts(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 效验参数
  ASSERT(wDataSize <= sizeof(CMD_GP_LogonAccounts));
  if (wDataSize > sizeof(CMD_GP_LogonAccounts))
    return false;

  // 变量定义
  WORD wBindIndex = LOWORD(dwSocketID);
  tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

  // 处理消息
  CMD_GP_LogonAccounts* pLogonAccounts = (CMD_GP_LogonAccounts*) pData;
  pLogonAccounts->szAccounts[std::size(pLogonAccounts->szAccounts) - 1] = 0;
  pLogonAccounts->szPassword[std::size(pLogonAccounts->szPassword) - 1] = 0;
  pLogonAccounts->szMachineID[std::size(pLogonAccounts->szMachineID) - 1] = 0;
  pLogonAccounts->szPassPortID[std::size(pLogonAccounts->szPassPortID) - 1] = 0;

  // 设置连接
  pBindParameter->cbClientKind = CLIENT_KIND_COMPUTER;
  pBindParameter->dwPlazaVersion = pLogonAccounts->dwPlazaVersion;

  // 版本判断
  if (CheckPlazaVersion(DEVICE_TYPE_PC, pLogonAccounts->dwPlazaVersion, dwSocketID,
                        ((pLogonAccounts->cbValidateFlags & LOW_VER_VALIDATE_FLAGS) != 0)) == false) {
    return true;
  }

  // 变量定义
  DBR_GP_LogonAccounts LogonAccounts;
  ZeroMemory(&LogonAccounts, sizeof(LogonAccounts));

  // 附加信息
  LogonAccounts.pBindParameter = (bind_parameter_ + LOWORD(dwSocketID));

  // 构造数据
  LogonAccounts.dwCheckUserRight = 0;
  LogonAccounts.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
  LogonAccounts.cbNeeValidateMBCard = (pLogonAccounts->cbValidateFlags & MB_VALIDATE_FLAGS);
  LSTRCPYN(LogonAccounts.szAccounts, pLogonAccounts->szAccounts, std::size(LogonAccounts.szAccounts));
  LSTRCPYN(LogonAccounts.szPassword, pLogonAccounts->szPassword, std::size(LogonAccounts.szPassword));
  LSTRCPYN(LogonAccounts.szMachineID, pLogonAccounts->szMachineID, std::size(LogonAccounts.szMachineID));
  LSTRCPYN(LogonAccounts.szPassPortID, pLogonAccounts->szPassPortID, std::size(LogonAccounts.szPassPortID));

  // 投递请求
  database_engine_->PostDataBaseRequest(DBR_GP_LOGON_ACCOUNTS, dwSocketID, &LogonAccounts, sizeof(LogonAccounts));

  return true;
}

// 工具登录
bool CDispatchEngineSink::OnTCPNetworkSubPCLogonManageTool(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 效验参数
  ASSERT(wDataSize >= sizeof(CMD_GP_LogonAccounts));
  if (wDataSize < sizeof(CMD_GP_LogonAccounts)) {
    if (wDataSize < sizeof(CMD_GP_LogonAccounts) - sizeof(BYTE))
      return false;
  }

  // 变量定义
  WORD wBindIndex = LOWORD(dwSocketID);
  tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

  // 处理消息
  CMD_GP_LogonAccounts* pLogonAccounts = (CMD_GP_LogonAccounts*) pData;
  pLogonAccounts->szAccounts[std::size(pLogonAccounts->szAccounts) - 1] = 0;
  pLogonAccounts->szPassword[std::size(pLogonAccounts->szPassword) - 1] = 0;
  pLogonAccounts->szMachineID[std::size(pLogonAccounts->szMachineID) - 1] = 0;

  // 设置连接
  pBindParameter->cbClientKind = CLIENT_KIND_COMPUTER;
  pBindParameter->dwPlazaVersion = pLogonAccounts->dwPlazaVersion;

  // 版本判断
  if (CheckPlazaVersion(DEVICE_TYPE_PC, pLogonAccounts->dwPlazaVersion, dwSocketID,
                        ((pLogonAccounts->cbValidateFlags & LOW_VER_VALIDATE_FLAGS) != 0)) == false) {
    return true;
  }

  // 变量定义
  DBR_GP_LogonAccounts LogonAccounts;
  ZeroMemory(&LogonAccounts, sizeof(LogonAccounts));

  // 附加信息
  LogonAccounts.pBindParameter = (bind_parameter_ + LOWORD(dwSocketID));

  // 构造数据
  LogonAccounts.dwCheckUserRight = UR_CAN_MANAGER_ANDROID;
  LogonAccounts.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
  LSTRCPYN(LogonAccounts.szAccounts, pLogonAccounts->szAccounts, std::size(LogonAccounts.szAccounts));
  LSTRCPYN(LogonAccounts.szPassword, pLogonAccounts->szPassword, std::size(LogonAccounts.szPassword));
  LSTRCPYN(LogonAccounts.szMachineID, pLogonAccounts->szMachineID, std::size(LogonAccounts.szMachineID));
  LogonAccounts.cbNeeValidateMBCard = (pLogonAccounts->cbValidateFlags & MB_VALIDATE_FLAGS);

  // 投递请求
  database_engine_->PostDataBaseRequest(DBR_GP_LOGON_ACCOUNTS, dwSocketID, &LogonAccounts, sizeof(LogonAccounts));

  return true;
}

// 帐号注册
bool CDispatchEngineSink::OnTCPNetworkSubPCRegisterAccounts(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 效验参数
  ASSERT(wDataSize >= sizeof(CMD_GP_RegisterAccounts));
  if (wDataSize < sizeof(CMD_GP_RegisterAccounts)) {
    if (wDataSize < sizeof(CMD_GP_RegisterAccounts) - sizeof(BYTE))
      return false;
  }

  // 变量定义
  WORD wBindIndex = LOWORD(dwSocketID);
  tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

  // 处理消息
  CMD_GP_RegisterAccounts* pRegisterAccounts = (CMD_GP_RegisterAccounts*) pData;
  pRegisterAccounts->szAccounts[std::size(pRegisterAccounts->szAccounts) - 1] = 0;
  pRegisterAccounts->szNickName[std::size(pRegisterAccounts->szNickName) - 1] = 0;
  pRegisterAccounts->szMachineID[std::size(pRegisterAccounts->szMachineID) - 1] = 0;
  pRegisterAccounts->szLogonPass[std::size(pRegisterAccounts->szLogonPass) - 1] = 0;
  pRegisterAccounts->szPassPortID[std::size(pRegisterAccounts->szPassPortID) - 1] = 0;
  pRegisterAccounts->szCompellation[std::size(pRegisterAccounts->szCompellation) - 1] = 0;

  // 设置连接
  pBindParameter->cbClientKind = CLIENT_KIND_COMPUTER;
  pBindParameter->dwPlazaVersion = pRegisterAccounts->dwPlazaVersion;

  // 效验版本
  if (CheckPlazaVersion(DEVICE_TYPE_PC, pRegisterAccounts->dwPlazaVersion, dwSocketID,
                        ((pRegisterAccounts->cbValidateFlags & LOW_VER_VALIDATE_FLAGS) != 0)) == false) {
    return true;
  }

  // 变量定义
  DBR_GP_RegisterAccounts RegisterAccounts;
  ZeroMemory(&RegisterAccounts, sizeof(RegisterAccounts));

  // 附加信息
  RegisterAccounts.pBindParameter = (bind_parameter_ + LOWORD(dwSocketID));

  // 构造数据
  RegisterAccounts.wFaceID = pRegisterAccounts->wFaceID;
  RegisterAccounts.cbGender = pRegisterAccounts->cbGender;
  RegisterAccounts.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
  RegisterAccounts.dwAgentID = pRegisterAccounts->dwAgentID;
  RegisterAccounts.dwSpreaderGameID = pRegisterAccounts->dwSpreaderGameID;
  LSTRCPYN(RegisterAccounts.szAccounts, pRegisterAccounts->szAccounts, std::size(RegisterAccounts.szAccounts));
  LSTRCPYN(RegisterAccounts.szNickName, pRegisterAccounts->szNickName, std::size(RegisterAccounts.szNickName));
  LSTRCPYN(RegisterAccounts.szMachineID, pRegisterAccounts->szMachineID, std::size(RegisterAccounts.szMachineID));
  LSTRCPYN(RegisterAccounts.szLogonPass, pRegisterAccounts->szLogonPass, std::size(RegisterAccounts.szLogonPass));
  LSTRCPYN(RegisterAccounts.szPassPortID, pRegisterAccounts->szPassPortID, std::size(RegisterAccounts.szPassPortID));
  LSTRCPYN(RegisterAccounts.szCompellation, pRegisterAccounts->szCompellation, std::size(RegisterAccounts.szCompellation));

  // 投递请求
  database_engine_->PostDataBaseRequest(DBR_GP_REGISTER_ACCOUNTS, dwSocketID, &RegisterAccounts, sizeof(RegisterAccounts));

  return true;
}

// 验证资料
bool CDispatchEngineSink::OnTCPNetworkSubPCVerifyIndividual(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 效验参数
  ASSERT(wDataSize >= sizeof(CMD_GP_VerifyIndividual));
  if (wDataSize < sizeof(CMD_GP_VerifyIndividual))
    return false;

  // 处理消息
  CMD_GP_VerifyIndividual* pVerifyIndividual = (CMD_GP_VerifyIndividual*) pData;

  // 效验版本
  if (CheckPlazaVersion(DEVICE_TYPE_PC, pVerifyIndividual->dwPlazaVersion, dwSocketID, true) == false) {
    return true;
  }

  // 变量定义
  DBR_GP_VerifyIndividual VerifyIndividual;
  ZeroMemory(&VerifyIndividual, sizeof(VerifyIndividual));

  // 设置变量
  VerifyIndividual.wVerifyMask = pVerifyIndividual->wVerifyMask;
  VerifyIndividual.pBindParameter = (bind_parameter_ + LOWORD(dwSocketID));

  // 变量定义
  VOID* pDataBuffer = nullptr;
  tagDataDescribe DataDescribe;
  CRecvPacketHelper RecvPacket(pVerifyIndividual + 1, wDataSize - sizeof(CMD_GP_VerifyIndividual));

  // 扩展信息
  while (true) {
    pDataBuffer = RecvPacket.GetData(DataDescribe);
    if (DataDescribe.wDataDescribe == DTP_NULL)
      break;

    if (DataDescribe.wDataDescribe == DTP_GP_UI_NICKNAME) // 用户昵称
    {
      ASSERT(pDataBuffer != nullptr);
      ASSERT(DataDescribe.wDataSize <= sizeof(VerifyIndividual.szVerifyContent));
      if (DataDescribe.wDataSize <= sizeof(VerifyIndividual.szVerifyContent)) {
        CopyMemory(&VerifyIndividual.szVerifyContent, pDataBuffer, DataDescribe.wDataSize);
        VerifyIndividual.szVerifyContent[std::size(VerifyIndividual.szVerifyContent) - 1] = 0;
      }
      break;
    }

    if (DataDescribe.wDataDescribe == DTP_GP_UI_ACCOUNTS) // 用户帐号
    {
      ASSERT(pDataBuffer != nullptr);
      ASSERT(DataDescribe.wDataSize <= sizeof(VerifyIndividual.szVerifyContent));
      if (DataDescribe.wDataSize <= sizeof(VerifyIndividual.szVerifyContent)) {
        CopyMemory(&VerifyIndividual.szVerifyContent, pDataBuffer, DataDescribe.wDataSize);
        VerifyIndividual.szVerifyContent[std::size(VerifyIndividual.szVerifyContent) - 1] = 0;
      }
      break;
    }
  }

  // 投递请求
  database_engine_->PostDataBaseRequest(DBR_GP_VERIFY_INDIVIDUAL, dwSocketID, &VerifyIndividual, sizeof(VerifyIndividual));

  return true;
}

// 游客登录
bool CDispatchEngineSink::OnTCPNetworkSubPCLogonVisitor(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 效验参数
  ASSERT(wDataSize >= sizeof(CMD_GP_LogonVisitor));
  if (wDataSize < sizeof(CMD_GP_LogonVisitor))
    return false;

  // 变量定义
  WORD wBindIndex = LOWORD(dwSocketID);
  tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

  // 处理消息
  CMD_GP_LogonVisitor* pLogonVisitor = (CMD_GP_LogonVisitor*) pData;

  // 设置连接
  pBindParameter->cbClientKind = CLIENT_KIND_COMPUTER;
  pBindParameter->dwPlazaVersion = pLogonVisitor->dwPlazaVersion;

  // 版本判断
  if (CheckPlazaVersion(DEVICE_TYPE_PC, pLogonVisitor->dwPlazaVersion, dwSocketID,
                        ((pLogonVisitor->cbValidateFlags & LOW_VER_VALIDATE_FLAGS) != 0)) == false) {
    return true;
  }

  // 变量定义
  DBR_GP_LogonVisitor LogonVisitor;
  ZeroMemory(&LogonVisitor, sizeof(LogonVisitor));

  // 附加信息
  LogonVisitor.pBindParameter = (bind_parameter_ + LOWORD(dwSocketID));

  // 构造数据
  LogonVisitor.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
  LogonVisitor.cbPlatformID = ULMByVisitor;
  LSTRCPYN(LogonVisitor.szMachineID, pLogonVisitor->szMachineID, std::size(LogonVisitor.szMachineID));

  // 投递请求
  database_engine_->PostDataBaseRequest(DBR_GP_LOGON_VISITOR, dwSocketID, &LogonVisitor, sizeof(LogonVisitor));

  return true;
}

// 查询验证码
bool CDispatchEngineSink::OnTCPNetworkSubPCQueryVerifyCode(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  ASSERT(wDataSize == sizeof(CMD_GP_QueryVerifyCode));
  if (wDataSize != sizeof(CMD_GP_QueryVerifyCode))
    return false;

  CMD_GP_QueryVerifyCode* pQueryVerifyCode = (CMD_GP_QueryVerifyCode*) pData;
  pQueryVerifyCode->szMachineID[std::size(pQueryVerifyCode->szMachineID) - 1] = 0;
  pQueryVerifyCode->szVerifyCode[std::size(pQueryVerifyCode->szVerifyCode) - 1] = 0;

  DWORD dwCurrentTime = (DWORD) time(nullptr);
  DWORD dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
  TCHAR szSendVerifyCode[LEN_VERIFY_CODE] = TEXT("");

  // 查找是否已申请
  INT i = 0;
  for (i = 0; i < m_VerifyCodeArray.GetCount(); i++) {
    tagVerifyCode* pVerifyCode = m_VerifyCodeArray[i];
    if (pVerifyCode == nullptr)
      continue;

    if (pVerifyCode->dwClientAddr == dwClientAddr && StrCmpT(pVerifyCode->szMachineID, pQueryVerifyCode->szMachineID) == 0) {
      if (dwCurrentTime - pVerifyCode->dwUpdateTime < TIME_VALID_VERIFY_CODE) {
        CMD_GP_VerifyCodeResult VerifyCodeResult;
        ZeroMemory(&VerifyCodeResult, sizeof(VerifyCodeResult));

        VerifyCodeResult.cbResultCode = 2;
        StrnCpyT(VerifyCodeResult.szDescString, TEXT("抱歉，您在短时间内查询次数过于频繁，请稍后重试！"), std::size(VerifyCodeResult.szDescString));

        // 发送消息
        network_engine_->SendData(dwSocketID, MDM_GP_LOGON, SUB_GP_VERIFY_CODE_RESULT, &VerifyCodeResult, sizeof(VerifyCodeResult));
        return true;
      }

      // 重新生成
      pVerifyCode->dwUpdateTime = dwCurrentTime;
      RandVerifyCode(pVerifyCode->szValidateCode, std::size(pVerifyCode->szValidateCode));
      LSTRCPYN(szSendVerifyCode, pVerifyCode->szValidateCode, std::size(szSendVerifyCode));
      break;
    }
  }

  // 新增请求项
  if (i == m_VerifyCodeArray.GetCount()) {
    tagVerifyCode* pNewVerifyCode = new tagVerifyCode;
    pNewVerifyCode->dwClientAddr = dwClientAddr;
    pNewVerifyCode->dwUpdateTime = dwCurrentTime;
    LSTRCPYN(pNewVerifyCode->szMachineID, pQueryVerifyCode->szMachineID, std::size(pNewVerifyCode->szMachineID));
    RandVerifyCode(pNewVerifyCode->szValidateCode, std::size(pNewVerifyCode->szValidateCode));
    m_VerifyCodeArray.Add(pNewVerifyCode);

    LSTRCPYN(szSendVerifyCode, pNewVerifyCode->szValidateCode, std::size(szSendVerifyCode));
  }

  // 发送验证码
  SendVerifyCode(dwSocketID, szSendVerifyCode, true);

  return true;
}

// I D 登录
bool CDispatchEngineSink::OnTCPNetworkSubMBLogonGameID(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 效验参数
  ASSERT(wDataSize >= sizeof(CMD_MB_LogonGameID));
  if (wDataSize < sizeof(CMD_MB_LogonGameID))
    return false;

  // 变量定义
  WORD wBindIndex = LOWORD(dwSocketID);
  tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

  // 处理消息
  CMD_MB_LogonGameID* pLogonGameID = (CMD_MB_LogonGameID*) pData;
  pLogonGameID->szPassword[std::size(pLogonGameID->szPassword) - 1] = 0;
  pLogonGameID->szMachineID[std::size(pLogonGameID->szMachineID) - 1] = 0;
  pLogonGameID->szMobilePhone[std::size(pLogonGameID->szMobilePhone) - 1] = 0;

  // 设置连接
  pBindParameter->cbClientKind = CLIENT_KIND_MOBILE;
  pBindParameter->wModuleID = pLogonGameID->wModuleID;
  pBindParameter->dwPlazaVersion = pLogonGameID->dwPlazaVersion;

  // 效验版本
  if (CheckPlazaVersion(pLogonGameID->cbDeviceType, pLogonGameID->dwPlazaVersion, dwSocketID) == false)
    return true;

  // 变量定义
  DBR_MB_LogonGameID LogonGameID;
  ZeroMemory(&LogonGameID, sizeof(LogonGameID));

  // 附加信息
  LogonGameID.pBindParameter = (bind_parameter_ + LOWORD(dwSocketID));

  // 构造数据
  LogonGameID.dwGameID = pLogonGameID->dwGameID;
  LogonGameID.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
  LSTRCPYN(LogonGameID.szPassword, pLogonGameID->szPassword, std::size(LogonGameID.szPassword));
  LSTRCPYN(LogonGameID.szMachineID, pLogonGameID->szMachineID, std::size(LogonGameID.szMachineID));
  LSTRCPYN(LogonGameID.szMobilePhone, pLogonGameID->szMobilePhone, std::size(LogonGameID.szMobilePhone));

  // 投递请求
  database_engine_->PostDataBaseRequest(DBR_MB_LOGON_GAMEID, dwSocketID, &LogonGameID, sizeof(LogonGameID));

  return true;
}

// 帐号登录
bool CDispatchEngineSink::OnTCPNetworkSubMBLogonAccounts(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 效验参数
  ASSERT(wDataSize >= sizeof(CMD_MB_LogonAccounts));
  if (wDataSize < sizeof(CMD_MB_LogonAccounts))
    return false;

  // 变量定义
  WORD wBindIndex = LOWORD(dwSocketID);
  tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

  // 处理消息
  CMD_MB_LogonAccounts* pLogonAccounts = (CMD_MB_LogonAccounts*) pData;
  pLogonAccounts->szAccounts[std::size(pLogonAccounts->szAccounts) - 1] = 0;
  pLogonAccounts->szPassword[std::size(pLogonAccounts->szPassword) - 1] = 0;
  pLogonAccounts->szMachineID[std::size(pLogonAccounts->szMachineID) - 1] = 0;
  pLogonAccounts->szMobilePhone[std::size(pLogonAccounts->szMobilePhone) - 1] = 0;

  // 设置连接
  pBindParameter->cbClientKind = CLIENT_KIND_MOBILE;
  pBindParameter->wModuleID = pLogonAccounts->wModuleID;
  pBindParameter->dwPlazaVersion = pLogonAccounts->dwPlazaVersion;

  // 版本判断
  if (CheckPlazaVersion(pLogonAccounts->cbDeviceType, pLogonAccounts->dwPlazaVersion, dwSocketID) == false)
    return true;

  // 变量定义
  DBR_MB_LogonAccounts LogonAccounts;
  ZeroMemory(&LogonAccounts, sizeof(LogonAccounts));

  // 附加信息
  LogonAccounts.pBindParameter = (bind_parameter_ + LOWORD(dwSocketID));

  // 构造数据
  LogonAccounts.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
  LSTRCPYN(LogonAccounts.szAccounts, pLogonAccounts->szAccounts, std::size(LogonAccounts.szAccounts));
  LSTRCPYN(LogonAccounts.szPassword, pLogonAccounts->szPassword, std::size(LogonAccounts.szPassword));
  LSTRCPYN(LogonAccounts.szMachineID, pLogonAccounts->szMachineID, std::size(LogonAccounts.szMachineID));
  LSTRCPYN(LogonAccounts.szMobilePhone, pLogonAccounts->szMobilePhone, std::size(LogonAccounts.szMobilePhone));

  // 投递请求
  database_engine_->PostDataBaseRequest(DBR_MB_LOGON_ACCOUNTS, dwSocketID, &LogonAccounts, sizeof(LogonAccounts));

  return true;
}

// 帐号注册
bool CDispatchEngineSink::OnTCPNetworkSubMBRegisterAccounts(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 效验参数
  ASSERT(wDataSize >= sizeof(CMD_MB_RegisterAccounts));
  if (wDataSize < sizeof(CMD_MB_RegisterAccounts))
    return false;

  // 变量定义
  WORD wBindIndex = LOWORD(dwSocketID);
  tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

  // 处理消息
  CMD_MB_RegisterAccounts* pRegisterAccounts = (CMD_MB_RegisterAccounts*) pData;
  pRegisterAccounts->szAccounts[std::size(pRegisterAccounts->szAccounts) - 1] = 0;
  pRegisterAccounts->szNickName[std::size(pRegisterAccounts->szNickName) - 1] = 0;
  pRegisterAccounts->szMachineID[std::size(pRegisterAccounts->szMachineID) - 1] = 0;
  pRegisterAccounts->szLogonPass[std::size(pRegisterAccounts->szLogonPass) - 1] = 0;
  pRegisterAccounts->szMobilePhone[std::size(pRegisterAccounts->szMobilePhone) - 1] = 0;

  // 设置连接
  pBindParameter->cbClientKind = CLIENT_KIND_MOBILE;
  pBindParameter->wModuleID = pRegisterAccounts->wModuleID;
  pBindParameter->dwPlazaVersion = pRegisterAccounts->dwPlazaVersion;

  // 效验版本
  if (CheckPlazaVersion(pRegisterAccounts->cbDeviceType, pRegisterAccounts->dwPlazaVersion, dwSocketID) == false)
    return true;

  // 变量定义
  DBR_MB_RegisterAccounts RegisterAccounts;
  ZeroMemory(&RegisterAccounts, sizeof(RegisterAccounts));

  // 附加信息
  RegisterAccounts.pBindParameter = (bind_parameter_ + LOWORD(dwSocketID));

  // 构造数据
  RegisterAccounts.cbDeviceType = pRegisterAccounts->cbDeviceType;
  RegisterAccounts.wFaceID = pRegisterAccounts->wFaceID;
  RegisterAccounts.cbGender = pRegisterAccounts->cbGender;
  RegisterAccounts.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
  RegisterAccounts.dwSpreaderGameID = pRegisterAccounts->dwSpreaderGameID;
  LSTRCPYN(RegisterAccounts.szAccounts, pRegisterAccounts->szAccounts, std::size(RegisterAccounts.szAccounts));
  LSTRCPYN(RegisterAccounts.szNickName, pRegisterAccounts->szNickName, std::size(RegisterAccounts.szNickName));
  LSTRCPYN(RegisterAccounts.szMachineID, pRegisterAccounts->szMachineID, std::size(RegisterAccounts.szMachineID));
  LSTRCPYN(RegisterAccounts.szLogonPass, pRegisterAccounts->szLogonPass, std::size(RegisterAccounts.szLogonPass));
  LSTRCPYN(RegisterAccounts.szMobilePhone, pRegisterAccounts->szMobilePhone, std::size(RegisterAccounts.szMobilePhone));

  // 投递请求
  database_engine_->PostDataBaseRequest(DBR_MB_REGISTER_ACCOUNTS, dwSocketID, &RegisterAccounts, sizeof(RegisterAccounts));

  return true;
}

// 其他登录
bool CDispatchEngineSink::OnTCPNetworkSubMBLogonOtherPlatform(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 效验参数
  ASSERT(wDataSize >= sizeof(CMD_MB_LogonOtherPlatform));
  if (wDataSize < sizeof(CMD_MB_LogonOtherPlatform))
    return false;

  // 变量定义
  WORD wBindIndex = LOWORD(dwSocketID);
  tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

  // 处理消息
  CMD_MB_LogonOtherPlatform* pLogonOtherPlatform = (CMD_MB_LogonOtherPlatform*) pData;
  pLogonOtherPlatform->szUserUin[std::size(pLogonOtherPlatform->szUserUin) - 1] = 0;
  pLogonOtherPlatform->szNickName[std::size(pLogonOtherPlatform->szNickName) - 1] = 0;
  pLogonOtherPlatform->szMachineID[std::size(pLogonOtherPlatform->szMachineID) - 1] = 0;
  pLogonOtherPlatform->szMobilePhone[std::size(pLogonOtherPlatform->szMobilePhone) - 1] = 0;
  pLogonOtherPlatform->szCompellation[std::size(pLogonOtherPlatform->szCompellation) - 1] = 0;

  // 平台判断
  ASSERT(pLogonOtherPlatform->cbPlatformID == ULMBySina || pLogonOtherPlatform->cbPlatformID == ULMByTencent ||
         pLogonOtherPlatform->cbPlatformID == ULMByRenRen);
  if (pLogonOtherPlatform->cbPlatformID != ULMBySina && pLogonOtherPlatform->cbPlatformID != ULMByTencent &&
      pLogonOtherPlatform->cbPlatformID != ULMByRenRen)
    return false;

  // 设置连接
  pBindParameter->cbClientKind = CLIENT_KIND_MOBILE;
  pBindParameter->wModuleID = pLogonOtherPlatform->wModuleID;
  pBindParameter->dwPlazaVersion = pLogonOtherPlatform->dwPlazaVersion;

  // 版本判断
  if (CheckPlazaVersion(pLogonOtherPlatform->cbDeviceType, pLogonOtherPlatform->dwPlazaVersion, dwSocketID) == false)
    return true;

  // 变量定义
  DBR_MB_LogonOtherPlatform LogonOtherPlatform;
  ZeroMemory(&LogonOtherPlatform, sizeof(LogonOtherPlatform));

  // 附加信息
  LogonOtherPlatform.pBindParameter = (bind_parameter_ + LOWORD(dwSocketID));

  // 构造数据
  LogonOtherPlatform.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
  LogonOtherPlatform.cbGender = pLogonOtherPlatform->cbGender;
  LogonOtherPlatform.cbPlatformID = pLogonOtherPlatform->cbPlatformID;
  LogonOtherPlatform.cbDeviceType = pLogonOtherPlatform->cbDeviceType;
  LSTRCPYN(LogonOtherPlatform.szUserUin, pLogonOtherPlatform->szUserUin, std::size(LogonOtherPlatform.szUserUin));
  LSTRCPYN(LogonOtherPlatform.szNickName, pLogonOtherPlatform->szNickName, std::size(LogonOtherPlatform.szNickName));
  LSTRCPYN(LogonOtherPlatform.szMachineID, pLogonOtherPlatform->szMachineID, std::size(LogonOtherPlatform.szMachineID));
  LSTRCPYN(LogonOtherPlatform.szMobilePhone, pLogonOtherPlatform->szMobilePhone, std::size(LogonOtherPlatform.szMobilePhone));
  LSTRCPYN(LogonOtherPlatform.szCompellation, pLogonOtherPlatform->szCompellation, std::size(LogonOtherPlatform.szCompellation));
  LSTRCPYN(LogonOtherPlatform.szDeviceToken, pLogonOtherPlatform->szDeviceToken, std::size(LogonOtherPlatform.szDeviceToken));
  LSTRCPYN(LogonOtherPlatform.strFaceUrl, pLogonOtherPlatform->strFaceUrl, std::size(LogonOtherPlatform.strFaceUrl));

  // 投递请求
  database_engine_->PostDataBaseRequest(DBR_MB_LOGON_OTHERPLATFORM, dwSocketID, &LogonOtherPlatform, sizeof(LogonOtherPlatform));

  return true;
}

// 游客登录
bool CDispatchEngineSink::OnTCPNetworkSubMBLogonVisitor(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 效验参数
  ASSERT(wDataSize >= sizeof(CMD_MB_LogonVisitor));
  if (wDataSize < sizeof(CMD_MB_LogonVisitor))
    return false;

  // 变量定义
  WORD wBindIndex = LOWORD(dwSocketID);
  tagBindParameter* pBindParameter = (bind_parameter_ + wBindIndex);

  // 处理消息
  CMD_MB_LogonVisitor* pLogonVisitor = (CMD_MB_LogonVisitor*) pData;

  // 设置连接
  pBindParameter->cbClientKind = CLIENT_KIND_MOBILE;
  pBindParameter->wModuleID = pLogonVisitor->wModuleID;
  pBindParameter->dwPlazaVersion = pLogonVisitor->dwPlazaVersion;

  // 版本判断
  if (CheckPlazaVersion(pLogonVisitor->cbDeviceType, pLogonVisitor->dwPlazaVersion, dwSocketID) == false) {
    return true;
  }

  // 变量定义
  DBR_MB_LogonVisitor LogonVisitor;
  ZeroMemory(&LogonVisitor, sizeof(LogonVisitor));

  // 附加信息
  LogonVisitor.pBindParameter = (bind_parameter_ + LOWORD(dwSocketID));

  // 构造数据
  LogonVisitor.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
  LogonVisitor.cbPlatformID = ULMByVisitor;
  LogonVisitor.cbDeviceType = pLogonVisitor->cbDeviceType;
  LSTRCPYN(LogonVisitor.szMachineID, pLogonVisitor->szMachineID, std::size(LogonVisitor.szMachineID));
  LSTRCPYN(LogonVisitor.szMobilePhone, pLogonVisitor->szMobilePhone, std::size(LogonVisitor.szMobilePhone));

  // 投递请求
  database_engine_->PostDataBaseRequest(DBR_MB_LOGON_VISITOR, dwSocketID, &LogonVisitor, sizeof(LogonVisitor));

  return true;
}

// 获取在线人数
bool CDispatchEngineSink::OnTCPNetworkSubMBGetOnline(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 变量定义
  CMD_MB_GetOnline* pGetOnline = (CMD_MB_GetOnline*) pData;
  WORD wHeadSize = (sizeof(CMD_MB_GetOnline) - sizeof(pGetOnline->wOnLineServerID));

  // 效验数据
  ASSERT((wDataSize >= wHeadSize) && (wDataSize == (wHeadSize + pGetOnline->wServerCount * sizeof(WORD))));
  if ((wDataSize < wHeadSize) || (wDataSize != (wHeadSize + pGetOnline->wServerCount * sizeof(WORD))))
    return false;

  // 变量定义
  CMD_MB_KindOnline KindOnline;
  CMD_MB_ServerOnline ServerOnline;
  ZeroMemory(&KindOnline, sizeof(KindOnline));
  ZeroMemory(&ServerOnline, sizeof(ServerOnline));

  // 获取类型
  CKindItemMap::iterator* KindPosition = nullptr;
  do {
    // 获取类型
    CGameKindItem* pGameKindItem = m_ServerListManager.EmunGameKindItem(KindPosition);

    // 设置变量
    if (pGameKindItem != nullptr) {
      WORD wKindIndex = KindOnline.wKindCount++;
      KindOnline.wCountInfo[wKindIndex].wKindOrServerID = pGameKindItem->m_GameKind.wKindID;
      KindOnline.wCountInfo[wKindIndex].dwOnLineCount = pGameKindItem->m_GameKind.dwOnLineCount;
      KindOnline.wCountInfo[wKindIndex].dwAndroidCount = pGameKindItem->m_GameKind.dwAndroidCount;
      KindOnline.wCountInfo[wKindIndex].dwSetCount = pGameKindItem->m_GameKind.dwSetCount;
    }

    // 溢出判断
    if (KindOnline.wKindCount >= std::size(KindOnline.wCountInfo)) {
      ASSERT(FALSE);
      break;
    }

  } while (KindPosition != nullptr);

  // 获取房间
  for (WORD i = 0; i < pGetOnline->wServerCount; i++) {
    // 获取房间
    WORD wServerID = pGetOnline->wOnLineServerID[i];
    CGameServerItem* pGameServerItem = m_ServerListManager.SearchGameServer(wServerID);

    // 设置变量
    if (pGameServerItem != nullptr) {
      WORD wServerIndex = ServerOnline.wServerCount++;
      ServerOnline.wCountInfo[wServerIndex].wKindOrServerID = wServerID;
      ServerOnline.wCountInfo[wServerIndex].dwOnLineCount = pGameServerItem->m_GameServer.dwOnLineCount;
      ServerOnline.wCountInfo[wServerIndex].dwAndroidCount = pGameServerItem->m_GameServer.dwAndroidCount;
      ServerOnline.wCountInfo[wServerIndex].dwSetCount = pGameServerItem->m_GameServer.dwSetPlayerCount;
    }

    // 溢出判断
    if (ServerOnline.wServerCount >= std::size(ServerOnline.wCountInfo)) {
      ASSERT(FALSE);
      break;
    }
  }

  // 类型在线
  if (KindOnline.wKindCount > 0) {
    WORD wHeadSize = sizeof(KindOnline) - sizeof(KindOnline.wCountInfo);
    WORD wSendSize = wHeadSize + KindOnline.wKindCount * (sizeof(KindOnline.wCountInfo[0]));
    network_engine_->SendData(dwSocketID, MDM_MB_SERVER_LIST, SUB_MB_KINE_ONLINE, &KindOnline, wSendSize);
  }

  // 房间在线
  if (ServerOnline.wServerCount > 0) {
    WORD wHeadSize = sizeof(ServerOnline) - sizeof(ServerOnline.wCountInfo);
    WORD wSendSize = wHeadSize + ServerOnline.wServerCount * (sizeof(KindOnline.wCountInfo[0]));
    network_engine_->SendData(dwSocketID, MDM_MB_SERVER_LIST, SUB_MB_SERVER_ONLINE, &ServerOnline, wSendSize);
  }

  return true;
}

// 查询房间
bool CDispatchEngineSink::OnTCPNetworkSubMBQueryGameServer(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 校验数据
  ASSERT(wDataSize == sizeof(CMD_MB_QueryGameServer));
  if (wDataSize != sizeof(CMD_MB_QueryGameServer))
    return false;

  CMD_MB_QueryGameServer* pGameServer = (CMD_MB_QueryGameServer*) pData;

  // 构造数据
  CMD_CS_C_QueryGameServer QueryGameServer;
  ZeroMemory(&QueryGameServer, sizeof(CMD_CS_C_QueryGameServer));
  QueryGameServer.dwUserID = pGameServer->dwUserID;
  QueryGameServer.dwKindID = pGameServer->dwKindID;
  QueryGameServer.cbIsJoinGame = pGameServer->cbIsJoinGame;
  QueryGameServer.dwSocketID = dwSocketID;
  QueryGameServer.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;

  // 发送数据
  if (personal_service_) {
    personal_service_->SendData(MDM_CS_SERVICE_INFO, SUB_CS_C_QUERY_GAME_SERVER, &QueryGameServer, sizeof(CMD_CS_C_QueryGameServer));
  }

  return true;
}

// 搜索房间桌号
bool CDispatchEngineSink::OnTCPNetworkSubMBSearchServerTable(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 校验数据
  ASSERT(wDataSize == sizeof(CMD_MB_SearchServerTable));
  if (wDataSize != sizeof(CMD_MB_SearchServerTable))
    return false;

  CMD_MB_SearchServerTable* pSearchServerTable = (CMD_MB_SearchServerTable*) pData;

  CMD_CS_C_SearchTable SearchTable;
  ZeroMemory(&SearchTable, sizeof(CMD_CS_C_SearchTable));

  SearchTable.dwSocketID = dwSocketID;
  SearchTable.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
  SearchTable.dwPersonalRoomID = pSearchServerTable->dwPersonalRoomID;

  if (personal_service_) {
    personal_service_->SendData(MDM_CS_SERVICE_INFO, SUB_CS_C_SEARCH_TABLE, &SearchTable, sizeof(CMD_CS_C_SearchTable));
  }
  return true;
}

// 解散搜索房间桌号
bool CDispatchEngineSink::OnTCPNetworkSubMBDissumeSearchServerTable(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // if (m_pIPersonalRoomServiceManager)
  //{
  //	return m_pIPersonalRoomServiceManager->OnTCPNetworkSubMBDissumeSearchServerTable(pData, wDataSize, dwSocketID,
  // m_pBindParameter, m_pITCPSocketService);
  // }

  // 校验数据
  ASSERT(wDataSize == sizeof(CMD_MB_SearchServerTable));
  if (wDataSize != sizeof(CMD_MB_SearchServerTable))
    return false;

  CMD_MB_SearchServerTable* pSearchServerTable = (CMD_MB_SearchServerTable*) pData;

  CMD_CS_C_SearchTable SearchTable;
  ZeroMemory(&SearchTable, sizeof(CMD_CS_C_SearchTable));

  SearchTable.dwSocketID = dwSocketID;
  SearchTable.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
  SearchTable.dwPersonalRoomID = pSearchServerTable->dwPersonalRoomID;

  if (personal_service_) {
    personal_service_->SendData(MDM_CS_SERVICE_INFO, SUB_CS_C_DISSUME_SEARCH_TABLE, &SearchTable, sizeof(CMD_CS_C_SearchTable));
  }

  return true;
}

// 约战房间配置
bool CDispatchEngineSink::OnTCPNetworkSubMBPersonalParameter(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // if (m_pIPersonalRoomServiceManager)
  //{
  //	return m_pIPersonalRoomServiceManager->OnTCPNetworkSubMBPersonalParameter(pData, wDataSize, dwSocketID,
  // database_engine_);
  // }

  // 校验数据
  ASSERT(wDataSize == sizeof(CMD_MB_GetPersonalParameter));
  if (wDataSize != sizeof(CMD_MB_GetPersonalParameter))
    return false;

  CMD_MB_GetPersonalParameter* pGetPersonalParameter = (CMD_MB_GetPersonalParameter*) pData;

  // 构造数据
  DBR_MB_GetPersonalParameter GetPersonalParameter;
  ZeroMemory(&GetPersonalParameter, sizeof(DBR_MB_GetPersonalParameter));
  GetPersonalParameter.dwKindID = pGetPersonalParameter->dwKindID;

  // 投递数据
  database_engine_->PostDataBaseRequest(DBR_MB_GET_PERSONAL_PARAMETER, dwSocketID, &GetPersonalParameter, sizeof(DBR_MB_GetPersonalParameter));

  return true;
}
// 查询私人房间定制配置
bool CDispatchEngineSink::OnTCPNetworkSubMBGetPersonalRoomRule(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  return true;
  // 校验数据
  ASSERT(wDataSize == sizeof(CMD_MB_GetPersonalRule));
  if (wDataSize != sizeof(CMD_MB_GetPersonalRule))
    return false;

  CMD_MB_GetPersonalRule* pGetPersonalRule = (CMD_MB_GetPersonalRule*) pData;
  // 构造数据
  DBR_MB_GetPersonalRule GetPersonalRule;
  ZeroMemory(&GetPersonalRule, sizeof(DBR_MB_GetPersonalRule));
  GetPersonalRule.dwServerID = pGetPersonalRule->dwServerID;
  // 投递数据
  database_engine_->PostDataBaseRequest(DBR_MB_GET_PERSONAL_RULE, dwSocketID, &GetPersonalRule, sizeof(DBR_MB_GetPersonalRule));

  return true;
}

// 约战房间密码
bool CDispatchEngineSink::OnTCPNetworkSubMBQueryRoomPassword(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 校验数据
  ASSERT(wDataSize == sizeof(CMD_MB_SearchServerTable));
  if (wDataSize != sizeof(CMD_MB_SearchServerTable))
    return false;

  CMD_MB_SearchServerTable* pSearchServerTable = (CMD_MB_SearchServerTable*) pData;

  CMD_CS_C_SearchTable SearchTable;
  ZeroMemory(&SearchTable, sizeof(CMD_CS_C_SearchTable));

  SearchTable.dwSocketID = dwSocketID;
  SearchTable.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
  SearchTable.dwPersonalRoomID = pSearchServerTable->dwPersonalRoomID;

  if (personal_service_) {
    personal_service_->SendData(MDM_CS_SERVICE_INFO, SUB_CS_S_QUERY_ROOM_PASSWORD, &SearchTable, sizeof(CMD_CS_C_SearchTable));
  }
  return true;
}

// 查询约战房间列表
bool CDispatchEngineSink::OnTCPNetworkSubMBQueryPersonalRoomList(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // if (m_pIPersonalRoomServiceManager)
  //{
  //	return m_pIPersonalRoomServiceManager->OnTCPNetworkSubMBQueryPersonalRoomList(pData, wDataSize, dwSocketID,
  // m_pITCPSocketService);
  // }

  // 校验数据
  ASSERT(wDataSize == sizeof(CMD_MB_QeuryPersonalRoomList));
  if (wDataSize != sizeof(CMD_MB_QeuryPersonalRoomList))
    return false;

  CMD_MB_QeuryPersonalRoomList* pQueryPersonalRoomList = (CMD_MB_QeuryPersonalRoomList*) pData;

  CMD_MB_SC_QeuryPersonalRoomList SC_QeuryPersonalRoomList;
  SC_QeuryPersonalRoomList.dwUserID = pQueryPersonalRoomList->dwUserID;
  SC_QeuryPersonalRoomList.dwSocketID = dwSocketID;

  // 构造数据
  if (personal_service_) {
    personal_service_->SendData(MDM_CS_SERVICE_INFO, SUB_CS_S_QUERY_PERSONAL_ROOM_LIST, &SC_QeuryPersonalRoomList,
                                sizeof(CMD_MB_SC_QeuryPersonalRoomList));
  }

  return true;
}

// 玩家请求房间成绩
bool CDispatchEngineSink::OnTCPNetworkSubQueryUserRoomScore(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // if (m_pIPersonalRoomServiceManager)
  //{
  //	return m_pIPersonalRoomServiceManager->OnTCPNetworkSubQueryUserRoomScore(pData, wDataSize, dwSocketID, database_engine_);
  // }

  // 校验数据
  ASSERT(wDataSize == sizeof(CMD_GR_QUERY_USER_ROOM_SCORE));
  if (wDataSize != sizeof(CMD_GR_QUERY_USER_ROOM_SCORE))
    return false;

  // 提取数据
  CMD_GR_QUERY_USER_ROOM_SCORE* pQueryUserRoomScore = (CMD_GR_QUERY_USER_ROOM_SCORE*) pData;
  ASSERT(pQueryUserRoomScore != nullptr);

  // 发送数据
  DBR_GR_QUERY_USER_ROOM_INFO QUERY_USER_ROOM_INFO;
  ZeroMemory(&QUERY_USER_ROOM_INFO, sizeof(DBR_GR_QUERY_USER_ROOM_INFO));
  QUERY_USER_ROOM_INFO.dwUserID = pQueryUserRoomScore->dwUserID; // 请求房间成绩的玩家

  // 投递数据
  database_engine_->PostDataBaseRequest(DBR_GR_QUERY_USER_ROOM_SCORE, dwSocketID, &QUERY_USER_ROOM_INFO, sizeof(DBR_GR_QUERY_USER_ROOM_INFO));

  return true;
}

// 玩家请求房间成绩
bool CDispatchEngineSink::OnTCPNetworkSubQueryPersonalRoomUserInfo(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 校验数据
  ASSERT(wDataSize == sizeof(CMD_MB_QueryPersonalRoomUserInfo));
  if (wDataSize != sizeof(CMD_MB_QueryPersonalRoomUserInfo))
    return false;

  // 提取数据
  CMD_MB_QueryPersonalRoomUserInfo* pQueryUserInfo = (CMD_MB_QueryPersonalRoomUserInfo*) pData;
  ASSERT(pQueryUserInfo != nullptr);

  // 发送数据
  DBR_GR_QUERY_PERSONAL_ROOM_USER_INFO USER_INFO;

  USER_INFO.dwUserID = pQueryUserInfo->dwUserID;

  // 投递数据
  database_engine_->PostDataBaseRequest(DBR_MB_QUERY_PERSONAL_ROOM_USER_INFO, dwSocketID, &USER_INFO, sizeof(DBR_GR_QUERY_PERSONAL_ROOM_USER_INFO));

  return true;
}

// 玩家请求房间成绩
bool CDispatchEngineSink::OnTCPNetworkSubRoomCardExchangeToScore(VOID* pData, WORD wDataSize, DWORD dwSocketID) {
  // 校验数据
  ASSERT(wDataSize == sizeof(CMD_GP_ExchangeScoreByRoomCard));
  if (wDataSize != sizeof(CMD_GP_ExchangeScoreByRoomCard))
    return false;

  // 发送数据
  // 构造结构
  CMD_GP_ExchangeScoreByRoomCard* pExchangeScore = (CMD_GP_ExchangeScoreByRoomCard*) pData;
  pExchangeScore->szMachineID[std::size(pExchangeScore->szMachineID) - 1] = 0;

  // 构造结构
  DBR_GP_ExchangeScoreByRoomCard ExchangeScore;
  ZeroMemory(&ExchangeScore, sizeof(ExchangeScore));

  // 设置变量
  ExchangeScore.dwUserID = pExchangeScore->dwUserID;
  ExchangeScore.lExchangeRoomCard = pExchangeScore->lRoomCard;
  ExchangeScore.dwClientAddr = (bind_parameter_ + LOWORD(dwSocketID))->dwClientAddr;
  LSTRCPYN(ExchangeScore.szMachineID, pExchangeScore->szMachineID, std::size(ExchangeScore.szMachineID));

  // 投递数据
  database_engine_->PostDataBaseRequest(DBR_MB_ROOM_CARD_EXCHANGE_TO_SCORE, dwSocketID, &ExchangeScore, sizeof(DBR_GP_ExchangeScoreByRoomCard));

  return true;
}

// 登录成功
bool CDispatchEngineSink::OnDBPCLogonSuccess(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  BYTE cbDataBuffer[SOCKET_TCP_PACKET];
  DBO_GP_LogonSuccess* pDBOLogonSuccess = (DBO_GP_LogonSuccess*) pData;
  CMD_GP_LogonSuccess* pCMDLogonSuccess = (CMD_GP_LogonSuccess*) cbDataBuffer;

  // 发送定义
  WORD wHeadSize = sizeof(CMD_GP_LogonSuccess);
  CSendPacketHelper SendPacket(cbDataBuffer + wHeadSize, sizeof(cbDataBuffer) - wHeadSize);

  // 设置变量
  ZeroMemory(pCMDLogonSuccess, sizeof(CMD_GP_LogonSuccess));

  // 构造数据
  pCMDLogonSuccess->wFaceID = pDBOLogonSuccess->wFaceID;
  pCMDLogonSuccess->cbGender = pDBOLogonSuccess->cbGender;
  pCMDLogonSuccess->dwGameID = pDBOLogonSuccess->dwGameID;
  pCMDLogonSuccess->dwUserID = pDBOLogonSuccess->dwUserID;
  pCMDLogonSuccess->dwCustomID = pDBOLogonSuccess->dwCustomID;
  pCMDLogonSuccess->dwExperience = pDBOLogonSuccess->dwExperience;
  pCMDLogonSuccess->lLoveLiness = pDBOLogonSuccess->lLoveLiness;
  pCMDLogonSuccess->cbMoorMachine = pDBOLogonSuccess->cbMoorMachine;
  LSTRCPYN(pCMDLogonSuccess->szAccounts, pDBOLogonSuccess->szAccounts, std::size(pCMDLogonSuccess->szAccounts));
  LSTRCPYN(pCMDLogonSuccess->szNickName, pDBOLogonSuccess->szNickName, std::size(pCMDLogonSuccess->szNickName));
  LSTRCPYN(pCMDLogonSuccess->szDynamicPass, pDBOLogonSuccess->szDynamicPass, std::size(pCMDLogonSuccess->szDynamicPass));

  // 用户成绩
  pCMDLogonSuccess->lUserScore = pDBOLogonSuccess->lUserScore;
  pCMDLogonSuccess->lUserIngot = pDBOLogonSuccess->lUserIngot;
  pCMDLogonSuccess->lUserInsure = pDBOLogonSuccess->lUserInsure;
  pCMDLogonSuccess->dUserBeans = pDBOLogonSuccess->dUserBeans;

  // 配置信息
  pCMDLogonSuccess->cbInsureEnabled = pDBOLogonSuccess->cbInsureEnabled;
  pCMDLogonSuccess->cbShowServerStatus = m_bShowServerStatus ? 1 : 0;
  pCMDLogonSuccess->cbIsAgent = pDBOLogonSuccess->cbIsAgent;

  // 会员信息
  if (pDBOLogonSuccess->cbMemberOrder != 0 || pDBOLogonSuccess->dwCheckUserRight != 0) {
    DTP_GP_MemberInfo MemberInfo;
    ZeroMemory(&MemberInfo, sizeof(MemberInfo));
    MemberInfo.cbMemberOrder = pDBOLogonSuccess->cbMemberOrder;
    MemberInfo.MemberOverDate = pDBOLogonSuccess->MemberOverDate;
    SendPacket.AddPacket(&MemberInfo, sizeof(MemberInfo), DTP_GP_MEMBER_INFO);
  }

  // 个性签名
  if (pDBOLogonSuccess->szUnderWrite[0] != 0) {
    SendPacket.AddPacket(pDBOLogonSuccess->szUnderWrite, CountStringBuffer(pDBOLogonSuccess->szUnderWrite), DTP_GP_UNDER_WRITE);
  }

  // 登录成功
  WORD wSendSize = SendPacket.GetDataSize() + sizeof(CMD_GP_LogonSuccess);
  network_engine_->SendData(dwContextID, MDM_GP_LOGON, SUB_GP_LOGON_SUCCESS, cbDataBuffer, wSendSize);

  // 发送列表
  if (init_parameter_->delay_list_ == FALSE || pDBOLogonSuccess->dwCheckUserRight == UR_CAN_MANAGER_ANDROID) {
    // 发送列表
    SendGameTypeInfo(dwContextID);
    SendGameKindInfo(dwContextID);
    SendGamePageInfo(dwContextID, INVALID_WORD);
    SendGameNodeInfo(dwContextID, INVALID_WORD);
    // SendGameServerInfo(dwContextID,INVALID_WORD);
    SendGameServerInfo(dwContextID, INVALID_WORD, DEVICE_TYPE_PC);
    network_engine_->SendData(dwContextID, MDM_GP_SERVER_LIST, SUB_GP_LIST_FINISH);
  } else {
    SendGameTypeInfo(dwContextID);
    SendGameKindInfo(dwContextID);
    SendGamePageInfo(dwContextID, 0);
    // SendGameServerInfo(dwContextID,INVALID_WORD);
    SendGameServerInfo(dwContextID, INVALID_WORD, DEVICE_TYPE_PC);
    network_engine_->SendData(dwContextID, MDM_GP_SERVER_LIST, SUB_GP_LIST_FINISH);
  }

  // 等级配置
  SendGrowLevelConfig(dwContextID);

  // 道具配置
  SendGamePropertyTypeInfo(dwContextID);
  SendGamePropertyRelatInfo(dwContextID);
  SendGamePropertyInfo(dwContextID);
  SendGamePropertySubInfo(dwContextID);
  network_engine_->SendData(dwContextID, MDM_GP_SERVER_LIST, SUB_GP_PROPERTY_FINISH);

  // 会员配置
  // SendMemberConfig(dwContextID);

  // 认证配置
  SendRealAuthConfig(dwContextID);

  // 登录完成
  CMD_GP_LogonFinish LogonFinish;
  ZeroMemory(&LogonFinish, sizeof(LogonFinish));
  LogonFinish.wIntermitTime = init_parameter_->intermittent_time_;
  LogonFinish.wOnLineCountTime = init_parameter_->online_count_time_;
  network_engine_->SendData(dwContextID, MDM_GP_LOGON, SUB_GP_LOGON_FINISH, &LogonFinish, sizeof(LogonFinish));

  return true;
}

// 登录失败
bool CDispatchEngineSink::OnDBPCLogonFailure(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  CMD_GP_LogonFailure LogonFailure;
  ZeroMemory(&LogonFailure, sizeof(LogonFailure));
  DBO_GP_LogonFailure* pLogonFailure = (DBO_GP_LogonFailure*) pData;

  // 构造数据
  LogonFailure.lResultCode = pLogonFailure->lResultCode;
  LSTRCPYN(LogonFailure.szDescribeString, pLogonFailure->szDescribeString, std::size(LogonFailure.szDescribeString));

  // 发送数据
  WORD wStringSize = CountStringBuffer(LogonFailure.szDescribeString);
  WORD wSendSize = sizeof(LogonFailure) - sizeof(LogonFailure.szDescribeString) + wStringSize;
  network_engine_->SendData(dwContextID, MDM_GP_LOGON, SUB_GP_LOGON_FAILURE, &LogonFailure, wSendSize);

  // 关闭连接
  network_engine_->ShutDownSocket(dwContextID);

  return true;
}

// 登录失败
bool CDispatchEngineSink::OnDBPCLogonValidateMBCard(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize == sizeof(DBO_GP_ValidateMBCard));
  if (wDataSize != sizeof(DBO_GP_ValidateMBCard))
    return false;

  DBO_GP_ValidateMBCard* pValidateMBCard = (DBO_GP_ValidateMBCard*) pData;

  // 构造结构
  CMD_GP_ValidateMBCard ValidateMBCard;
  ValidateMBCard.uMBCardID = pValidateMBCard->uMBCardID;

  // 发送消息
  network_engine_->SendData(dwContextID, MDM_GP_LOGON, SUB_GP_VALIDATE_MBCARD, &ValidateMBCard, sizeof(ValidateMBCard));

  return true;
}

// 登录失败
bool CDispatchEngineSink::OnDBPCLogonValidatePassPort(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 发送消息
  network_engine_->SendData(dwContextID, MDM_GP_LOGON, SUB_GP_VALIDATE_PASSPORT, nullptr, 0);

  return true;
}

// 验证结果
bool CDispatchEngineSink::OnDBPCLogonVerifyResult(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize <= sizeof(DBO_GP_VerifyIndividualResult));
  if (wDataSize > sizeof(DBO_GP_VerifyIndividualResult))
    return false;

  // 提取数据
  DBO_GP_VerifyIndividualResult* pVerifyIndividualResult = (DBO_GP_VerifyIndividualResult*) pData;

  // 构造结构
  CMD_GP_VerifyIndividualResult VerifyIndividualResult;
  VerifyIndividualResult.wVerifyMask = pVerifyIndividualResult->wVerifyMask;
  VerifyIndividualResult.bVerifyPassage = pVerifyIndividualResult->bVerifyPassage;
  LSTRCPYN(VerifyIndividualResult.szErrorMsg, pVerifyIndividualResult->szErrorMsg, std::size(VerifyIndividualResult.szErrorMsg));

  // 发送数据
  WORD wStringSize = CountStringBuffer(VerifyIndividualResult.szErrorMsg);
  WORD wSendSize = sizeof(VerifyIndividualResult) - sizeof(VerifyIndividualResult.szErrorMsg) + wStringSize;
  network_engine_->SendData(dwContextID, MDM_GP_LOGON, SUB_GP_VERIFY_RESULT, &VerifyIndividualResult, wSendSize);

  return true;
}

// 平台配置
bool CDispatchEngineSink::OnDBPCPlatformParameter(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 变量定义
  DBO_GP_PlatformParameter* pPlatformParameter = (DBO_GP_PlatformParameter*) pData;

  // 设置变量
  m_PlatformParameter.dwExchangeRate = pPlatformParameter->dwExchangeRate;
  m_PlatformParameter.dwPresentExchangeRate = pPlatformParameter->dwPresentExchangeRate;
  m_PlatformParameter.dwRateGold = pPlatformParameter->dwRateGold;

  return true;
}

// 用户头像
bool CDispatchEngineSink::OnDBPCUserFaceInfo(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  CMD_GP_UserFaceInfo UserFaceInfo;
  ZeroMemory(&UserFaceInfo, sizeof(UserFaceInfo));
  DBO_GP_UserFaceInfo* pUserFaceInfo = (DBO_GP_UserFaceInfo*) pData;

  // 设置变量
  UserFaceInfo.wFaceID = pUserFaceInfo->wFaceID;
  UserFaceInfo.dwCustomID = pUserFaceInfo->dwCustomID;

  // 发送消息
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_USER_FACE_INFO, &UserFaceInfo, sizeof(UserFaceInfo));

  return true;
}

// 用户信息
bool CDispatchEngineSink::OnDBPCUserIndividual(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  BYTE cbDataBuffer[SOCKET_TCP_PACKET];
  DBO_GP_UserIndividual* pDBOUserIndividual = (DBO_GP_UserIndividual*) pData;
  CMD_GP_UserIndividual* pCMDUserIndividual = (CMD_GP_UserIndividual*) cbDataBuffer;
  CSendPacketHelper SendPacket(cbDataBuffer + sizeof(CMD_GP_UserIndividual), sizeof(cbDataBuffer) - sizeof(CMD_GP_UserIndividual));

  // 设置变量
  ZeroMemory(pCMDUserIndividual, sizeof(CMD_GP_UserIndividual));

  // 构造数据
  pCMDUserIndividual->dwUserID = pDBOUserIndividual->dwUserID;

  // 用户信息
  if (pDBOUserIndividual->szUserNote[0] != 0) {
    WORD wBufferSize = CountStringBuffer(pDBOUserIndividual->szUserNote);
    SendPacket.AddPacket(pDBOUserIndividual->szUserNote, wBufferSize, DTP_GP_UI_USER_NOTE);
  }

  // 真实
  if (pDBOUserIndividual->szCompellation[0] != 0) {
    WORD wBufferSize = CountStringBuffer(pDBOUserIndividual->szCompellation);
    SendPacket.AddPacket(pDBOUserIndividual->szCompellation, wBufferSize, DTP_GP_UI_COMPELLATION);
  }

  // 身份证
  if (pDBOUserIndividual->szPassPortID[0] != 0) {
    WORD wBufferSize = CountStringBuffer(pDBOUserIndividual->szPassPortID);
    SendPacket.AddPacket(pDBOUserIndividual->szPassPortID, wBufferSize, DTP_GP_UI_PASSPORTID);
  }

  // 电话号码
  if (pDBOUserIndividual->szSeatPhone[0] != 0) {
    WORD wBufferSize = CountStringBuffer(pDBOUserIndividual->szSeatPhone);
    SendPacket.AddPacket(pDBOUserIndividual->szSeatPhone, wBufferSize, DTP_GP_UI_SEAT_PHONE);
  }

  // 移动电话
  if (pDBOUserIndividual->szMobilePhone[0] != 0) {
    WORD wBufferSize = CountStringBuffer(pDBOUserIndividual->szMobilePhone);
    SendPacket.AddPacket(pDBOUserIndividual->szMobilePhone, wBufferSize, DTP_GP_UI_MOBILE_PHONE);
  }

  // 联系资料
  if (pDBOUserIndividual->szQQ[0] != 0) {
    WORD wBufferSize = CountStringBuffer(pDBOUserIndividual->szQQ);
    SendPacket.AddPacket(pDBOUserIndividual->szQQ, wBufferSize, DTP_GP_UI_QQ);
  }

  // 电子邮件
  if (pDBOUserIndividual->szEMail[0] != 0) {
    WORD wBufferSize = CountStringBuffer(pDBOUserIndividual->szEMail);
    SendPacket.AddPacket(pDBOUserIndividual->szEMail, wBufferSize, DTP_GP_UI_EMAIL);
  }

  // 联系地址
  if (pDBOUserIndividual->szDwellingPlace[0] != 0) {
    WORD wBufferSize = CountStringBuffer(pDBOUserIndividual->szDwellingPlace);
    SendPacket.AddPacket(pDBOUserIndividual->szDwellingPlace, wBufferSize, DTP_GP_UI_DWELLING_PLACE);
  }

  // 推广信息
  if (pDBOUserIndividual->szSpreader[0] != 0) {
    WORD wBufferSize = CountStringBuffer(pDBOUserIndividual->szSpreader);
    SendPacket.AddPacket(pDBOUserIndividual->szSpreader, wBufferSize, DTP_GP_UI_SPREADER);
  }

  // 发送消息
  WORD wSendSize = sizeof(CMD_GP_UserIndividual) + SendPacket.GetDataSize();
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_USER_INDIVIDUAL, cbDataBuffer, wSendSize);

  return true;
}
// 绑定推广
bool CDispatchEngineSink::OnDBUserBindSpreaderResult(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;
  // 变量定义
  DBO_GP_BindSpreaderResult* pBindSpreaderResult = (DBO_GP_BindSpreaderResult*) pData;
  // 变量定义
  CMD_GP_BindSpreaderResult BindSpreaderResult;
  ZeroMemory(&BindSpreaderResult, sizeof(BindSpreaderResult));

  // 构造数据
  BindSpreaderResult.dwDiamond = pBindSpreaderResult->dwDiamond;
  BindSpreaderResult.dwRewardDiamond = pBindSpreaderResult->dwRewardDiamond;
  LSTRCPYN(BindSpreaderResult.szDescribeString, pBindSpreaderResult->szDescribeString, std::size(BindSpreaderResult.szDescribeString));

  // 发送数据
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_BIND_SPREADER_RESULT, &BindSpreaderResult, sizeof(BindSpreaderResult));

  return true;
}

// 银行信息
bool CDispatchEngineSink::OnDBPCUserInsureInfo(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_UserInsureInfo* pUserInsureInfo = (DBO_GP_UserInsureInfo*) pData;

  // 变量定义
  CMD_GP_UserInsureInfo UserInsureInfo;
  ZeroMemory(&UserInsureInfo, sizeof(UserInsureInfo));

  // 构造数据
  UserInsureInfo.cbEnjoinTransfer = pUserInsureInfo->cbEnjoinTransfer;
  UserInsureInfo.wRevenueTake = pUserInsureInfo->wRevenueTake;
  UserInsureInfo.wRevenueTransfer = pUserInsureInfo->wRevenueTransfer;
  UserInsureInfo.wRevenueTransferMember = pUserInsureInfo->wRevenueTransferMember;
  UserInsureInfo.wServerID = pUserInsureInfo->wServerID;
  UserInsureInfo.lUserScore = pUserInsureInfo->lUserScore;
  UserInsureInfo.lUserInsure = pUserInsureInfo->lUserInsure;
  UserInsureInfo.lTransferPrerequisite = pUserInsureInfo->lTransferPrerequisite;
  UserInsureInfo.dwUserRight = pUserInsureInfo->dwUserRight;

  // 发送数据
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_USER_INSURE_INFO, &UserInsureInfo, sizeof(UserInsureInfo));

  // 关闭连接
  network_engine_->ShutDownSocket(dwContextID);

  return true;
}

// 银行成功
bool CDispatchEngineSink::OnDBPCUserInsureSuccess(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_UserInsureSuccess* pUserInsureSuccess = (DBO_GP_UserInsureSuccess*) pData;

  // 变量定义
  CMD_GP_UserInsureSuccess UserInsureSuccess;
  ZeroMemory(&UserInsureSuccess, sizeof(UserInsureSuccess));

  // 设置变量
  UserInsureSuccess.dwUserID = pUserInsureSuccess->dwUserID;
  UserInsureSuccess.lUserScore = pUserInsureSuccess->lSourceScore + pUserInsureSuccess->lVariationScore;
  UserInsureSuccess.lUserInsure = pUserInsureSuccess->lSourceInsure + pUserInsureSuccess->lVariationInsure + pUserInsureSuccess->lInsureRevenue;
  LSTRCPYN(UserInsureSuccess.szDescribeString, pUserInsureSuccess->szDescribeString, std::size(UserInsureSuccess.szDescribeString));

  // 发送消息
  WORD wDescribe = CountStringBuffer(UserInsureSuccess.szDescribeString);
  WORD wHeadSize = sizeof(UserInsureSuccess) - sizeof(UserInsureSuccess.szDescribeString);
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_USER_INSURE_SUCCESS, &UserInsureSuccess, wHeadSize + wDescribe);

  return true;
}

// 操作失败
bool CDispatchEngineSink::OnDBPCUserInsureFailure(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  CMD_GP_UserInsureFailure UserInsureFailure;
  ZeroMemory(&UserInsureFailure, sizeof(UserInsureFailure));

  // 变量定义
  DBO_GP_UserInsureFailure* pUserInsureFailure = (DBO_GP_UserInsureFailure*) pData;

  // 构造数据
  UserInsureFailure.lResultCode = pUserInsureFailure->lResultCode;
  LSTRCPYN(UserInsureFailure.szDescribeString, pUserInsureFailure->szDescribeString, std::size(UserInsureFailure.szDescribeString));

  // 发送数据
  WORD wDescribe = CountStringBuffer(UserInsureFailure.szDescribeString);
  WORD wHeadSize = sizeof(UserInsureFailure) - sizeof(UserInsureFailure.szDescribeString);
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_USER_INSURE_FAILURE, &UserInsureFailure, wHeadSize + wDescribe);

  // 关闭连接
  network_engine_->ShutDownSocket(dwContextID);

  return true;
}

// 用户信息
bool CDispatchEngineSink::OnDBPCUserInsureUserInfo(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_UserTransferUserInfo* pTransferUserInfo = (DBO_GP_UserTransferUserInfo*) pData;

  // 变量定义
  CMD_GP_UserTransferUserInfo UserTransferUserInfo;
  ZeroMemory(&UserTransferUserInfo, sizeof(UserTransferUserInfo));

  // 构造变量
  UserTransferUserInfo.dwTargetGameID = pTransferUserInfo->dwGameID;
  LSTRCPYN(UserTransferUserInfo.szAccounts, pTransferUserInfo->szAccounts, std::size(UserTransferUserInfo.szAccounts));

  // 发送数据
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_QUERY_USER_INFO_RESULT, &UserTransferUserInfo, sizeof(UserTransferUserInfo));

  return true;
}

// 开通结果
bool CDispatchEngineSink::OnDBPCUserInsureEnableResult(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_UserInsureEnableResult* pUserInsureEnableResult = (DBO_GP_UserInsureEnableResult*) pData;

  // 变量定义
  CMD_GP_UserInsureEnableResult UserInsureEnableResult;
  ZeroMemory(&UserInsureEnableResult, sizeof(UserInsureEnableResult));

  // 构造变量
  UserInsureEnableResult.cbInsureEnabled = pUserInsureEnableResult->cbInsureEnabled;
  LSTRCPYN(UserInsureEnableResult.szDescribeString, pUserInsureEnableResult->szDescribeString, std::size(UserInsureEnableResult.szDescribeString));

  // 发送数据
  WORD wHeadSize = CountStringBuffer(UserInsureEnableResult.szDescribeString);
  wHeadSize += sizeof(UserInsureEnableResult) - sizeof(UserInsureEnableResult.szDescribeString);
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_USER_INSURE_ENABLE_RESULT, &UserInsureEnableResult, wHeadSize);

  return true;
}

bool CDispatchEngineSink::OnDBPCQueryTransferRebateResult(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_QueryTransferRebateResult* pRebateResult = (DBO_GP_QueryTransferRebateResult*) pData;

  // 变量定义
  CMD_GP_QueryTransferRebateResult RebateResult;
  ZeroMemory(&RebateResult, sizeof(RebateResult));

  RebateResult.dwUserID = pRebateResult->dwUserID;
  RebateResult.cbRebateEnabled = pRebateResult->cbRebateEnabled;
  RebateResult.lIngot = pRebateResult->lIngot;
  RebateResult.lLoveLiness = pRebateResult->lLoveLiness;

  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_QUERY_TRANSFER_REBATE_RESULT, &RebateResult, sizeof(RebateResult));
  return true;
}

// 签到信息
bool CDispatchEngineSink::OnDBPCUserCheckInInfo(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_CheckInInfo* pCheckInInfo = (DBO_GP_CheckInInfo*) pData;

  // 变量定义
  CMD_GP_CheckInInfo CheckInInfo;
  ZeroMemory(&CheckInInfo, sizeof(CheckInInfo));

  // 构造变量
  CheckInInfo.bTodayChecked = pCheckInInfo->bTodayChecked;
  CheckInInfo.wSeriesDate = pCheckInInfo->wSeriesDate;
  CopyMemory(CheckInInfo.lRewardGold, m_lCheckInReward, sizeof(CheckInInfo.lRewardGold));

  // 发送数据
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_CHECKIN_INFO, &CheckInInfo, sizeof(CheckInInfo));

  return true;
}

// 签到结果
bool CDispatchEngineSink::OnDBPCUserCheckInResult(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_CheckInResult* pCheckInResult = (DBO_GP_CheckInResult*) pData;

  // 变量定义
  CMD_GP_CheckInResult CheckInResult;
  ZeroMemory(&CheckInResult, sizeof(CheckInResult));

  // 构造变量
  CheckInResult.bSuccessed = pCheckInResult->bSuccessed;
  CheckInResult.lScore = pCheckInResult->lScore;
  LSTRCPYN(CheckInResult.szNotifyContent, pCheckInResult->szNotifyContent, std::size(CheckInResult.szNotifyContent));

  // 发送数据
  WORD wSendSize = sizeof(CheckInResult) - sizeof(CheckInResult.szNotifyContent) + CountStringBuffer(CheckInResult.szNotifyContent);
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_CHECKIN_RESULT, &CheckInResult, wSendSize);

  return true;
}

// 签到奖励
bool CDispatchEngineSink::OnDBPCCheckInReward(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 参数校验
  ASSERT(wDataSize == sizeof(DBO_GP_CheckInReward));
  if (wDataSize != sizeof(DBO_GP_CheckInReward))
    return false;

  // 提取数据
  DBO_GP_CheckInReward* pCheckInReward = (DBO_GP_CheckInReward*) pData;

  // 拷贝数据
  CopyMemory(m_lCheckInReward, pCheckInReward->lRewardGold, sizeof(m_lCheckInReward));

  return true;
}

// 任务列表
bool CDispatchEngineSink::OnDBPCTaskList(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 变量定义
  DBO_GP_TaskListInfo* pTaskListInfo = (DBO_GP_TaskListInfo*) pData;

  // 拷贝数据
  WORD wTaskCount = std::min(pTaskListInfo->wTaskCount, WORD(std::size(m_TaskParameterBuffer) - m_wTaskCountBuffer));
  CopyMemory(m_TaskParameterBuffer + m_wTaskCountBuffer, ++pTaskListInfo, sizeof(tagTaskParameter) * wTaskCount);

  // 设置变量
  m_wTaskCountBuffer += wTaskCount;

  return true;
}

// 任务结束
bool CDispatchEngineSink::OnDBPCTaskListEnd(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 拷贝数据
  m_wTaskCount = m_wTaskCountBuffer;
  CopyMemory(m_TaskParameter, m_TaskParameterBuffer, sizeof(tagTaskParameter) * m_wTaskCount);

  return true;
}

// 任务信息
bool CDispatchEngineSink::OnDBPCTaskInfo(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_TaskInfo* pTaskInfo = (DBO_GP_TaskInfo*) pData;

  // 变量定义
  WORD wTaskCount = m_wTaskCount;

  // 网络数据
  WORD wSendSize = 0, wParameterSize = 0;
  BYTE cbDataBuffer[SOCKET_TCP_PACKET];
  uint8_t* pDataBuffer = cbDataBuffer;

  // 设置数量
  pDataBuffer += sizeof(wTaskCount);
  wSendSize = wSendSize;

  // 分包发送
  for (WORD i = 0; i < wTaskCount; i++) {
    // 越界判断
    if (wSendSize + sizeof(tagTaskParameter) > SOCKET_TCP_PACKET) {
      // 重置变量
      pDataBuffer = cbDataBuffer;
      wTaskCount -= i;
      *(WORD*) pDataBuffer = i;

      // 发送数据
      network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_TASK_LIST, &cbDataBuffer, wSendSize);

      // 重置变量
      pDataBuffer += sizeof(wSendSize);
      wSendSize = wSendSize;
    }

    // 计算大小
    wParameterSize = sizeof(tagTaskParameter) - sizeof(m_TaskParameter[i].szTaskDescribe) + CountStringBuffer(m_TaskParameter[i].szTaskDescribe);
    *(WORD*) pDataBuffer = wParameterSize;
    pDataBuffer += sizeof(wParameterSize);
    wSendSize += sizeof(wParameterSize);

    // 拷贝数据
    CopyMemory(pDataBuffer, &m_TaskParameter[i], wParameterSize);

    // 推进指针
    pDataBuffer += wParameterSize;
    wSendSize += wParameterSize;
  }

  // 剩余发送
  if (wTaskCount > 0 && wSendSize) {
    // 重置变量
    pDataBuffer = cbDataBuffer;
    *(WORD*) pDataBuffer = wTaskCount;

    // 发送数据
    network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_TASK_LIST, &cbDataBuffer, wSendSize);
  }

  // 构造结构
  CMD_GP_TaskInfo TaskInfo;
  TaskInfo.wTaskCount = pTaskInfo->wTaskCount;
  CopyMemory(TaskInfo.TaskStatus, pTaskInfo->TaskStatus, sizeof(TaskInfo.TaskStatus[0]) * pTaskInfo->wTaskCount);

  // 发送数据
  WORD wSendDataSize = sizeof(TaskInfo) - sizeof(TaskInfo.TaskStatus);
  wSendDataSize += sizeof(TaskInfo.TaskStatus[0]) * TaskInfo.wTaskCount;
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_TASK_INFO, &TaskInfo, wSendDataSize);

  return true;
}

// 任务结果
bool CDispatchEngineSink::OnDBPCTaskResult(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_TaskResult* pTaskResult = (DBO_GP_TaskResult*) pData;

  // 变量定义
  CMD_GP_TaskResult TaskResult;
  ZeroMemory(&TaskResult, sizeof(TaskResult));

  // 构造变量
  TaskResult.bSuccessed = pTaskResult->bSuccessed;
  TaskResult.wCommandID = pTaskResult->wCommandID;
  TaskResult.lCurrScore = pTaskResult->lCurrScore;
  TaskResult.lCurrIngot = pTaskResult->lCurrIngot;
  LSTRCPYN(TaskResult.szNotifyContent, pTaskResult->szNotifyContent, std::size(TaskResult.szNotifyContent));

  // 发送数据
  WORD wSendSize = sizeof(TaskResult) - sizeof(TaskResult.szNotifyContent) + CountStringBuffer(TaskResult.szNotifyContent);
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_TASK_RESULT, &TaskResult, wSendSize);

  return true;
}
// 视频列表
bool CDispatchEngineSink::OnDBMBVideoList(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 发送数据
  network_engine_->SendData(dwContextID, MDM_MB_VIDEO_PLAY_BACK_INFO, SUB_MB_QUERY_VIDEO_INFO_RESULT, pData, wDataSize);

  return true;
}

// 视频结束
bool CDispatchEngineSink::OnDBMBVideoListEnd(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 发送数据
  network_engine_->SendData(dwContextID, MDM_MB_VIDEO_PLAY_BACK_INFO, SUB_MB_QUERY_VIDEO_INFO_END, pData, wDataSize);

  return true;
}

// 视频详情
bool CDispatchEngineSink::OnDBMBVideoDetails(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 发送数据
  network_engine_->SendData(dwContextID, MDM_MB_VIDEO_PLAY_BACK_INFO, SUB_MB_QUERY_VIDEO_DETAILS_RESULT, pData, wDataSize);

  return true;
}

// 详情结束
bool CDispatchEngineSink::OnDBMBVideoDetailsEnd(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 发送数据
  network_engine_->SendData(dwContextID, MDM_MB_VIDEO_PLAY_BACK_INFO, SUB_MB_QUERY_VIDEO_DETAILS_END, pData, wDataSize);

  return true;
}
// 回放码
bool CDispatchEngineSink::OnDBMBPlayBackCodeYZ(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 参数校验
  ASSERT(wDataSize == sizeof(DBR_MB_QueryPlayBackCode_YZ_Result));
  if (wDataSize != sizeof(DBR_MB_QueryPlayBackCode_YZ_Result))
    return false;

  // 变量定义
  DBR_MB_QueryPlayBackCode_YZ_Result* pCodeParameter = (DBR_MB_QueryPlayBackCode_YZ_Result*) pData;

  // 构造结构
  CMD_MB_S_QueryPlayBackCode_YZ_Result CodeResult;
  CodeResult.dwUserID = pCodeParameter->dwUserID;
  CodeResult.dwPlayBackCode = pCodeParameter->dwPlayBackCode;
  LSTRCPYN(CodeResult.szPersonalGUID, pCodeParameter->szPersonalGUID, std::size(CodeResult.szPersonalGUID));

  // 发送数据
  network_engine_->SendData(dwContextID, MDM_MB_VIDEO_PLAY_BACK_INFO, SUB_MB_QUERY_PLAYBACK_CODE_YZ_RESULT, &CodeResult, sizeof(CodeResult));

  return true;
}
// 回放码
bool CDispatchEngineSink::OnDBMBPlayBackCode(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 参数校验
  ASSERT(wDataSize == sizeof(DBR_MB_QueryPlayBackCode_Result));
  if (wDataSize != sizeof(DBR_MB_QueryPlayBackCode_Result))
    return false;

  // 变量定义
  DBR_MB_QueryPlayBackCode_Result* pCodeParameter = (DBR_MB_QueryPlayBackCode_Result*) pData;

  // 构造结构
  CMD_MB_S_QueryPlayBackCode_Result CodeResult;
  CodeResult.dwUserID = pCodeParameter->dwUserID;
  CodeResult.dwPlayBackCode = pCodeParameter->dwPlayBackCode;
  LSTRCPYN(CodeResult.szVideoNumber, pCodeParameter->szVideoNumber, std::size(CodeResult.szVideoNumber));

  // 发送数据
  network_engine_->SendData(dwContextID, MDM_MB_VIDEO_PLAY_BACK_INFO, SUB_MB_QUERY_PLAYBACK_CODE_RESULT, &CodeResult, sizeof(CodeResult));

  return true;
}

// 低保参数
bool CDispatchEngineSink::OnDBPCBaseEnsureParameter(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 参数校验
  ASSERT(wDataSize == sizeof(DBO_GP_BaseEnsureParameter));
  if (wDataSize != sizeof(DBO_GP_BaseEnsureParameter))
    return false;

  // 变量定义
  DBO_GP_BaseEnsureParameter* pEnsureParameter = (DBO_GP_BaseEnsureParameter*) pData;

  // 设置变量
  m_BaseEnsureParameter.cbTakeTimes = pEnsureParameter->cbTakeTimes;
  m_BaseEnsureParameter.lScoreAmount = pEnsureParameter->lScoreAmount;
  m_BaseEnsureParameter.lScoreCondition = pEnsureParameter->lScoreCondition;

  return true;
}

// 低保结果
bool CDispatchEngineSink::OnDBPCBaseEnsureResult(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_BaseEnsureResult* pBaseEnsureResult = (DBO_GP_BaseEnsureResult*) pData;

  // 构造结构
  CMD_GP_BaseEnsureResult BaseEnsureResult;
  BaseEnsureResult.bSuccessed = pBaseEnsureResult->bSuccessed;
  BaseEnsureResult.lGameScore = pBaseEnsureResult->lGameScore;
  LSTRCPYN(BaseEnsureResult.szNotifyContent, pBaseEnsureResult->szNotifyContent, std::size(BaseEnsureResult.szNotifyContent));

  // 发送数据
  WORD wSendDataSize = sizeof(BaseEnsureResult) - sizeof(BaseEnsureResult.szNotifyContent);
  wSendDataSize += CountStringBuffer(BaseEnsureResult.szNotifyContent);
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_BASEENSURE_RESULT, &BaseEnsureResult, wSendDataSize);

  return true;
}

// 推广参数
bool CDispatchEngineSink::OnDBPCUserSpreadInfo(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_UserSpreadInfo* pSpreadParameter = (DBO_GP_UserSpreadInfo*) pData;

  // 构造结构
  CMD_GP_UserSpreadInfo UserSpreadInfo;
  UserSpreadInfo.dwSpreadCount = pSpreadParameter->dwSpreadCount;
  UserSpreadInfo.lSpreadReward = pSpreadParameter->lSpreadReward;

  // 发送数据
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_SPREAD_INFO, &UserSpreadInfo, sizeof(UserSpreadInfo));

  return true;
}

// 实名奖励
bool CDispatchEngineSink::OnDBPCRealAuthParameter(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 变量定义
  DBO_GP_RealAuthParameter* pParameter = (DBO_GP_RealAuthParameter*) pData;

  m_AuthRealParameter.dwAuthRealAward = pParameter->dwAuthRealAward;
  m_AuthRealParameter.dwAuthentDisable = pParameter->dwAuthentDisable;

  return true;
}

// 等级配置
bool CDispatchEngineSink::OnDBPCGrowLevelConfig(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 变量定义
  DBO_GP_GrowLevelConfig* pGrowLevelConfig = (DBO_GP_GrowLevelConfig*) pData;

  // 拷贝数据
  m_wLevelCount = pGrowLevelConfig->wLevelCount;
  CopyMemory(m_GrowLevelConfig, &pGrowLevelConfig->GrowLevelConfig, sizeof(tagGrowLevelConfig) * m_wLevelCount);

  return true;
}

// 等级参数
bool CDispatchEngineSink::OnDBPCGrowLevelParameter(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_GrowLevelParameter* pGrowLevelParameter = (DBO_GP_GrowLevelParameter*) pData;

  // 构造结构
  CMD_GP_GrowLevelParameter GrowLevelParameter;
  GrowLevelParameter.wCurrLevelID = pGrowLevelParameter->wCurrLevelID;
  GrowLevelParameter.dwExperience = pGrowLevelParameter->dwExperience;
  GrowLevelParameter.dwUpgradeExperience = pGrowLevelParameter->dwUpgradeExperience;
  GrowLevelParameter.lUpgradeRewardGold = pGrowLevelParameter->lUpgradeRewardGold;
  GrowLevelParameter.lUpgradeRewardIngot = pGrowLevelParameter->lUpgradeRewardIngot;

  // 发送数据
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_GROWLEVEL_PARAMETER, &GrowLevelParameter, sizeof(GrowLevelParameter));

  return true;
}

// 等级升级
bool CDispatchEngineSink::OnDBPCGrowLevelUpgrade(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_GrowLevelUpgrade* pGrowLevelUpgrade = (DBO_GP_GrowLevelUpgrade*) pData;

  // 构造结构
  CMD_GP_GrowLevelUpgrade GrowLevelUpgrade;
  GrowLevelUpgrade.lCurrScore = pGrowLevelUpgrade->lCurrScore;
  GrowLevelUpgrade.lCurrIngot = pGrowLevelUpgrade->lCurrIngot;
  LSTRCPYN(GrowLevelUpgrade.szNotifyContent, pGrowLevelUpgrade->szNotifyContent, std::size(GrowLevelUpgrade.szNotifyContent));

  // 发送数据
  WORD wSendDataSize = sizeof(GrowLevelUpgrade) - sizeof(GrowLevelUpgrade.szNotifyContent);
  wSendDataSize += CountStringBuffer(GrowLevelUpgrade.szNotifyContent);
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_GROWLEVEL_UPGRADE, &GrowLevelUpgrade, wSendDataSize);

  return true;
}

// 道具信息
bool CDispatchEngineSink::OnDBGamePropertyRelatItem(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize % sizeof(DBO_GP_GamePropertyRelatItem) == 0);
  if (wDataSize % sizeof(DBO_GP_GamePropertyRelatItem) != 0)
    return false;

  // 变量定义
  WORD wItemCount = wDataSize / sizeof(DBO_GP_GamePropertyRelatItem);
  DBO_GP_GamePropertyRelatItem* pGamePropertyRelatItem = (DBO_GP_GamePropertyRelatItem*) pData;

  // 更新数据
  for (WORD i = 0; i < wItemCount; i++) {
    // 变量定义
    tagPropertyRelatItem GamePropertyRelat;
    ZeroMemory(&GamePropertyRelat, sizeof(GamePropertyRelat));

    // 构造数据
    GamePropertyRelat.dwTypeID = (pGamePropertyRelatItem + i)->dwTypeID;
    GamePropertyRelat.dwPropertyID = (pGamePropertyRelatItem + i)->dwPropertyID;

    // 插入列表
    m_GamePropertyListManager.InsertGamePropertyRelatItem(&GamePropertyRelat);
  }

  return true;
}

// 道具信息
bool CDispatchEngineSink::OnDBGamePropertyTypeItem(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize % sizeof(DBO_GP_GamePropertyTypeItem) == 0);
  if (wDataSize % sizeof(DBO_GP_GamePropertyTypeItem) != 0)
    return false;

  // 变量定义
  WORD wItemCount = wDataSize / sizeof(DBO_GP_GamePropertyTypeItem);
  DBO_GP_GamePropertyTypeItem* pGamePropertyTypeItem = (DBO_GP_GamePropertyTypeItem*) pData;

  // 更新数据
  for (WORD i = 0; i < wItemCount; i++) {
    // 变量定义
    tagPropertyTypeItem GamePropertyType;
    ZeroMemory(&GamePropertyType, sizeof(GamePropertyType));

    // 构造数据
    GamePropertyType.dwTypeID = (pGamePropertyTypeItem + i)->dwTypeID;
    GamePropertyType.dwSortID = (pGamePropertyTypeItem + i)->dwSortID;
    LSTRCPYN(GamePropertyType.szTypeName, (pGamePropertyTypeItem + i)->szTypeName, std::size(GamePropertyType.szTypeName));

    // 插入列表
    m_GamePropertyListManager.InsertGamePropertyTypeItem(&GamePropertyType);
  }

  return true;
}
// 道具信息
bool CDispatchEngineSink::OnDBGamePropertyItem(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize % sizeof(DBO_GP_GamePropertyItem) == 0);
  if (wDataSize % sizeof(DBO_GP_GamePropertyItem) != 0)
    return false;

  // 变量定义
  WORD wItemCount = wDataSize / sizeof(DBO_GP_GamePropertyItem);
  DBO_GP_GamePropertyItem* pGamePropertyItem = (DBO_GP_GamePropertyItem*) pData;

  // 查询存在
  for (WORD nNewIndex = 0; nNewIndex < wItemCount; nNewIndex++) {
    DBO_GP_GamePropertyItem* pCurrGamePropertyItem = (DBO_GP_GamePropertyItem*) (pGamePropertyItem + nNewIndex);

    // 变量定义
    tagPropertyItem GameProperty;
    ZeroMemory(&GameProperty, sizeof(GameProperty));

    GameProperty.dwPropertyID = pCurrGamePropertyItem->dwPropertyID;
    GameProperty.dwPropertyKind = pCurrGamePropertyItem->dwPropertyKind;
    GameProperty.cbUseArea = pCurrGamePropertyItem->cbUseArea;
    GameProperty.cbServiceArea = pCurrGamePropertyItem->cbServiceArea;
    GameProperty.cbRecommend = pCurrGamePropertyItem->cbRecommend;
    GameProperty.dwExchangeRatio = pCurrGamePropertyItem->dwExchangeRatio;
    /*GameProperty.lPropertyGold = pCurrGamePropertyItem->lPropertyGold;
                GameProperty.dPropertyCash = pCurrGamePropertyItem->dPropertyCash;
                GameProperty.lPropertyUserMedal = pCurrGamePropertyItem->lPropertyUserMedal;
                GameProperty.lPropertyLoveLiness = pCurrGamePropertyItem->lPropertyLoveLiness;*/
    GameProperty.lSendLoveLiness = pCurrGamePropertyItem->lSendLoveLiness;
    GameProperty.lRecvLoveLiness = pCurrGamePropertyItem->lRecvLoveLiness;
    GameProperty.lUseResultsGold = pCurrGamePropertyItem->lUseResultsGold;
    GameProperty.dwUseResultsValidTime = pCurrGamePropertyItem->dwUseResultsValidTime;
    GameProperty.dwUseResultsValidTimeScoreMultiple = pCurrGamePropertyItem->dwUseResultsValidTimeScoreMultiple;
    GameProperty.dwUseResultsGiftPackage = pCurrGamePropertyItem->dwUseResultsGiftPackage;
    GameProperty.dwSortID = pCurrGamePropertyItem->dwSortID;
    LSTRCPYN(GameProperty.szName, pCurrGamePropertyItem->szName, std::size(GameProperty.szName));
    LSTRCPYN(GameProperty.szRegulationsInfo, pCurrGamePropertyItem->szRegulationsInfo, std::size(GameProperty.szRegulationsInfo));

    // 插入列表
    m_GamePropertyListManager.InsertGamePropertyItem(&GameProperty);
  }

  return true;
}

// 道具信息
bool CDispatchEngineSink::OnDBGamePropertySubItem(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize % sizeof(DBO_GP_GamePropertySubItem) == 0);
  if (wDataSize % sizeof(DBO_GP_GamePropertySubItem) != 0)
    return false;

  // 变量定义
  WORD wItemCount = wDataSize / sizeof(DBO_GP_GamePropertySubItem);
  DBO_GP_GamePropertySubItem* pGamePropertySubtem = (DBO_GP_GamePropertySubItem*) pData;

  // 更新数据
  for (WORD i = 0; i < wItemCount; i++) {
    // 变量定义
    tagPropertySubItem GamePropertySub;
    ZeroMemory(&GamePropertySub, sizeof(GamePropertySub));

    // 构造数据
    GamePropertySub.dwPropertyID = (pGamePropertySubtem + i)->dwPropertyID;
    GamePropertySub.dwPropertyCount = (pGamePropertySubtem + i)->dwPropertyCount;
    GamePropertySub.dwOwnerPropertyID = (pGamePropertySubtem + i)->dwOwnerPropertyID;
    GamePropertySub.dwSortID = (pGamePropertySubtem + i)->dwSortID;

    // 插入列表
    m_GamePropertyListManager.InsertGamePropertySubItem(&GamePropertySub);
  }

  return true;
}

// 道具结果
bool CDispatchEngineSink::OnDBGamePropertyListResult(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 变量定义
  DBO_GP_GamePropertyListResult* pGamePropertyInfo = (DBO_GP_GamePropertyListResult*) pData;

  // 设置道具
  if (pGamePropertyInfo->cbSuccess == TRUE) {
  }

  return true;
}
// 道具购买
bool CDispatchEngineSink::OnDBGamePropertyFailure(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_PropertyFailure* pPropertyFailure = (DBO_GP_PropertyFailure*) pData;

  // 构造结构
  CMD_GP_PropertyFailure PropertyFailure;
  PropertyFailure.lErrorCode = pPropertyFailure->lErrorCode;
  LSTRCPYN(PropertyFailure.szDescribeString, pPropertyFailure->szDescribeString, std::size(PropertyFailure.szDescribeString));
  network_engine_->SendData(dwContextID, MDM_GP_PROPERTY, SUB_GP_PROPERTY_FAILURE, &PropertyFailure, sizeof(PropertyFailure));

  return true;
}

bool CDispatchEngineSink::OnDBQueryPropertySingle(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_PropertyQuerySingle* pPropertyBuyResult = (DBO_GP_PropertyQuerySingle*) pData;

  // 构造结构
  CMD_GP_PropertyQuerySingleResult PropertyBuyResult;
  PropertyBuyResult.dwUserID = pPropertyBuyResult->dwUserID;
  PropertyBuyResult.dwPropertyID = pPropertyBuyResult->dwPropertyID;
  PropertyBuyResult.dwItemCount = pPropertyBuyResult->dwItemCount;
  network_engine_->SendData(dwContextID, MDM_GP_PROPERTY, SUB_GP_QUERY_SINGLE_RESULT, &PropertyBuyResult, sizeof(PropertyBuyResult));
  return true;
}

// 道具购买
bool CDispatchEngineSink::OnDBGamePropertyBuy(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_PropertyBuyResult* pPropertyBuyResult = (DBO_GP_PropertyBuyResult*) pData;

  // 构造结构
  CMD_GP_PropertyBuyResult PropertyBuyResult;
  PropertyBuyResult.dwUserID = pPropertyBuyResult->dwUserID;
  PropertyBuyResult.dwPropertyID = pPropertyBuyResult->dwPropertyID;
  PropertyBuyResult.dwItemCount = pPropertyBuyResult->dwItemCount;
  PropertyBuyResult.lDiamond = pPropertyBuyResult->lDiamond;
  //	PropertyBuyResult.lInsureScore = pPropertyBuyResult->lInsureScore;
  //	PropertyBuyResult.lUserMedal = pPropertyBuyResult->lUserMedal;
  //	PropertyBuyResult.lLoveLiness = pPropertyBuyResult->lLoveLiness;
  //	PropertyBuyResult.dCash = pPropertyBuyResult->dCash;
  //	PropertyBuyResult.cbCurrMemberOrder = pPropertyBuyResult->cbCurrMemberOrder;
  LSTRCPYN(PropertyBuyResult.szNotifyContent, pPropertyBuyResult->szNotifyContent, std::size(PropertyBuyResult.szNotifyContent));
  network_engine_->SendData(dwContextID, MDM_GP_PROPERTY, SUB_GP_PROPERTY_BUY_RESULT, &PropertyBuyResult, sizeof(PropertyBuyResult));

  return true;
}

bool CDispatchEngineSink::OnDBQueryUserBackpack(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  DBO_GP_QueryBackpack* pQueryBackpackResult = (DBO_GP_QueryBackpack*) pData;

  // 构造返回
  BYTE cbDataBuffer[SOCKET_TCP_PACKET] = {0};
  CMD_GP_S_BackpackProperty* pBackpackPropertyResult = (CMD_GP_S_BackpackProperty*) cbDataBuffer;

  // 初始化参数
  WORD dwDataBufferSize = sizeof(CMD_GP_S_BackpackProperty);
  pBackpackPropertyResult->dwUserID = pQueryBackpackResult->dwUserID;
  pBackpackPropertyResult->dwCount = pQueryBackpackResult->dwCount;
  pBackpackPropertyResult->dwStatus = pQueryBackpackResult->dwStatus;
  if (pQueryBackpackResult->dwCount > 0) {
    memcpy(pBackpackPropertyResult->PropertyInfo, pQueryBackpackResult->PropertyInfo, sizeof(tagBackpackProperty) * (pQueryBackpackResult->dwCount));
    dwDataBufferSize += sizeof(tagBackpackProperty) * (WORD) (pQueryBackpackResult->dwCount - 1);
  }
  // 发生数据
  network_engine_->SendData(dwContextID, MDM_GP_PROPERTY, SUB_GP_QUERY_BACKPACKET_RESULT, pBackpackPropertyResult, dwDataBufferSize);
  return true;
}

bool CDispatchEngineSink::OnDBUserPropertyBuff(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  DBO_GR_UserPropertyBuff* pUserPropertyBuff = (DBO_GR_UserPropertyBuff*) pData;

  CMD_GP_S_UserPropertyBuff UserPropertyBuff;
  UserPropertyBuff.dwUserID = pUserPropertyBuff->dwUserID;
  UserPropertyBuff.cbBuffCount = pUserPropertyBuff->cbBuffCount;
  memcpy(UserPropertyBuff.PropertyBuff, pUserPropertyBuff->PropertyBuff, sizeof(tagPropertyBuff) * pUserPropertyBuff->cbBuffCount);

  WORD dwHeadSize = sizeof(UserPropertyBuff) - sizeof(UserPropertyBuff.PropertyBuff);
  WORD dwDataSize = UserPropertyBuff.cbBuffCount * sizeof(UserPropertyBuff.PropertyBuff[0]);

  // 发送数据
  network_engine_->SendData(dwContextID, MDM_GP_PROPERTY, SUB_GP_PROPERTY_BUFF_RESULT, &UserPropertyBuff, dwHeadSize + dwDataSize);
  return true;
}

bool CDispatchEngineSink::OnDBGamePropertyUse(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  DBO_GP_PropertyUse* pPropertyUseResult = (DBO_GP_PropertyUse*) pData;

  // 构造结构
  CMD_GP_S_PropertyUse PropertyUseResult;

  PropertyUseResult.dwUserID = pPropertyUseResult->dwUserID;
  PropertyUseResult.dwRecvUserID = pPropertyUseResult->dwRecvUserID;
  PropertyUseResult.dwPropID = pPropertyUseResult->dwPropID;
  PropertyUseResult.dwPropKind = pPropertyUseResult->dwPropKind;
  PropertyUseResult.wPropCount = pPropertyUseResult->wPropCount;
  PropertyUseResult.Score = pPropertyUseResult->Score;
  PropertyUseResult.dwRemainderPropCount = pPropertyUseResult->dwRemainderPropCount;
  PropertyUseResult.dwScoreMultiple = pPropertyUseResult->dwScoreMultiple;
  PropertyUseResult.lRecvLoveLiness = pPropertyUseResult->lRecvLoveLiness;
  PropertyUseResult.lSendLoveLiness = pPropertyUseResult->lSendLoveLiness;
  PropertyUseResult.lUseResultsGold = pPropertyUseResult->lUseResultsGold;
  PropertyUseResult.UseResultsValidTime = pPropertyUseResult->UseResultsValidTime;
  PropertyUseResult.tUseTime = pPropertyUseResult->tUseTime;
  PropertyUseResult.dwHandleCode = pPropertyUseResult->dwHandleCode;
  PropertyUseResult.cbMemberOrder = pPropertyUseResult->cbMemberOrder;
  LSTRCPYN(PropertyUseResult.szName, pPropertyUseResult->szName, std::size(PropertyUseResult.szName));
  LSTRCPYN(PropertyUseResult.szNotifyContent, pPropertyUseResult->szNotifyContent, std::size(PropertyUseResult.szNotifyContent));
  WORD wSendSize = sizeof(PropertyUseResult) - sizeof(PropertyUseResult.szNotifyContent) + CountStringBuffer(PropertyUseResult.szNotifyContent);

  network_engine_->SendData(dwContextID, MDM_GP_PROPERTY, SUB_GP_PROPERTY_USE_RESULT, &PropertyUseResult, wSendSize);

  return true;
}

bool CDispatchEngineSink::OnDBUserPropertyPresent(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  DBO_GP_PropertyPresent* pPropertyPresent = (DBO_GP_PropertyPresent*) pData;

  // 构造结构
  CMD_GP_S_PropertyPresent PropertyPresentResult = {0};
  PropertyPresentResult.dwUserID = pPropertyPresent->dwUserID;
  PropertyPresentResult.dwRecvGameID = pPropertyPresent->dwRecvGameID;
  PropertyPresentResult.dwPropID = pPropertyPresent->dwPropID;
  PropertyPresentResult.wPropCount = pPropertyPresent->wPropCount;
  PropertyPresentResult.wType = pPropertyPresent->wType;
  PropertyPresentResult.nHandleCode = pPropertyPresent->nHandleCode;

  LSTRCPYN(PropertyPresentResult.szRecvNickName, pPropertyPresent->szRecvNickName, std::size(PropertyPresentResult.szRecvNickName));
  LSTRCPYN(PropertyPresentResult.szNotifyContent, pPropertyPresent->szNotifyContent, std::size(PropertyPresentResult.szNotifyContent));

  network_engine_->SendData(dwContextID, MDM_GP_PROPERTY, SUB_GP_PROPERTY_PRESENT_RESULT, &PropertyPresentResult, sizeof(PropertyPresentResult));
  return true;
}

bool CDispatchEngineSink::OnDBQuerySendPresent(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  DBO_GP_QuerySendPresent* pQuerySendPresent = (DBO_GP_QuerySendPresent*) pData;
  CMD_GP_S_QuerySendPresent QuerySendPresentResult = {0};
  QuerySendPresentResult.wPresentCount = pQuerySendPresent->wPresentCount;
  memcpy(QuerySendPresentResult.Present, pQuerySendPresent->Present,
         sizeof(pQuerySendPresent->wPresentCount * sizeof(pQuerySendPresent->Present[0])));

  WORD dwHeadSize = sizeof(QuerySendPresentResult) - sizeof(QuerySendPresentResult.Present);
  WORD dwDataSize = QuerySendPresentResult.wPresentCount * sizeof(QuerySendPresentResult.Present[0]);
  network_engine_->SendData(dwContextID, MDM_GP_PROPERTY, SUB_GP_QUERY_SEND_PRESENT_RESULT, &QuerySendPresentResult, dwHeadSize + dwDataSize);
  return true;
}

bool CDispatchEngineSink::OnDBGetSendPresent(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  DBO_GP_GetSendPresent* pGetSendPresent = (DBO_GP_GetSendPresent*) pData;
  CMD_GP_S_GetSendPresent GetSendPresentResult = {0};
  GetSendPresentResult.wPresentCount = pGetSendPresent->wPresentCount;
  memcpy(GetSendPresentResult.Present, pGetSendPresent->Present, pGetSendPresent->wPresentCount * sizeof(SendPresent));

  WORD dwHeadSize = sizeof(GetSendPresentResult) - sizeof(GetSendPresentResult.Present);
  WORD dwDataSize = GetSendPresentResult.wPresentCount * sizeof(GetSendPresentResult.Present[0]);
  network_engine_->SendData(dwContextID, MDM_GP_PROPERTY, SUB_GP_GET_SEND_PRESENT_RESULT, &GetSendPresentResult, dwHeadSize + dwDataSize);
  return true;
}

// 会员参数
bool CDispatchEngineSink::OnDBPCMemberParameter(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 变量定义
  DBO_GP_MemberParameter* pMemberParameter = (DBO_GP_MemberParameter*) pData;

  // 拷贝数据
  m_wMemberCount = pMemberParameter->wMemberCount;
  CopyMemory(m_MemberParameter, pMemberParameter->MemberParameter, sizeof(tagMemberParameterNew) * m_wMemberCount);

  return true;
}

// 会员查询
bool CDispatchEngineSink::OnDBPCMemberDayQueryInfoResult(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_MemberQueryInfoResult* pMemberResult = (DBO_GP_MemberQueryInfoResult*) pData;

  // 构造结构
  CMD_GP_MemberQueryInfoResult MemberResult;
  MemberResult.bPresent = pMemberResult->bPresent;
  MemberResult.bGift = pMemberResult->bGift;
  // 拷贝数据
  MemberResult.GiftSubCount = pMemberResult->GiftSubCount;
  CopyMemory(MemberResult.GiftSub, pMemberResult->GiftSub, sizeof(tagGiftPropertyInfo) * MemberResult.GiftSubCount);

  // 发送数据
  // 计算大小
  WORD wSendDataSize = sizeof(MemberResult) - sizeof(MemberResult.GiftSub);
  wSendDataSize += sizeof(tagGiftPropertyInfo) * (WORD) (MemberResult.GiftSubCount);

  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_MEMBER_QUERY_INFO_RESULT, &MemberResult, wSendDataSize);

  return true;
}

// 会员送金
bool CDispatchEngineSink::OnDBPCMemberDayPresentResult(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_MemberDayPresentResult* pMemberResult = (DBO_GP_MemberDayPresentResult*) pData;

  // 构造结构
  CMD_GP_MemberDayPresentResult MemberResult;
  MemberResult.bSuccessed = pMemberResult->bSuccessed;
  MemberResult.lGameScore = pMemberResult->lGameScore;
  LSTRCPYN(MemberResult.szNotifyContent, pMemberResult->szNotifyContent, std::size(MemberResult.szNotifyContent));

  // 发送数据
  WORD wSendDataSize = sizeof(MemberResult) - sizeof(MemberResult.szNotifyContent);
  wSendDataSize += CountStringBuffer(MemberResult.szNotifyContent);
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_MEMBER_DAY_PRESENT_RESULT, &MemberResult, wSendDataSize);

  return true;
}

// 会员礼包
bool CDispatchEngineSink::OnDBPCMemberDayGiftResult(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_MemberDayGiftResult* pMemberResult = (DBO_GP_MemberDayGiftResult*) pData;

  // 构造结构
  CMD_GP_MemberDayGiftResult MemberResult;
  MemberResult.bSuccessed = pMemberResult->bSuccessed;
  LSTRCPYN(MemberResult.szNotifyContent, pMemberResult->szNotifyContent, std::size(MemberResult.szNotifyContent));

  // 发送数据
  WORD wSendDataSize = sizeof(MemberResult) - sizeof(MemberResult.szNotifyContent);
  wSendDataSize += CountStringBuffer(MemberResult.szNotifyContent);
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_MEMBER_DAY_GIFT_RESULT, &MemberResult, wSendDataSize);
  return true;
}

// 购买结果
bool CDispatchEngineSink::OnDBPCPurchaseResult(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 提取数据
  DBO_GP_PurchaseResult* pPurchaseResult = (DBO_GP_PurchaseResult*) pData;

  // 构造结构
  CMD_GP_PurchaseResult PurchaseResult;
  ZeroMemory(&PurchaseResult, sizeof(PurchaseResult));

  // 设置变量
  PurchaseResult.bSuccessed = pPurchaseResult->bSuccessed;
  PurchaseResult.lCurrScore = pPurchaseResult->lCurrScore;
  PurchaseResult.dCurrBeans = pPurchaseResult->dCurrBeans;
  PurchaseResult.cbMemberOrder = pPurchaseResult->cbMemberOrder;
  LSTRCPYN(PurchaseResult.szNotifyContent, pPurchaseResult->szNotifyContent, std::size(PurchaseResult.szNotifyContent));

  // 发送数据
  WORD wSendDataSize = sizeof(PurchaseResult) - sizeof(PurchaseResult.szNotifyContent);
  wSendDataSize += CountStringBuffer(PurchaseResult.szNotifyContent);
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_PURCHASE_RESULT, &PurchaseResult, wSendDataSize);

  return true;
}

// 房卡兑换结果
bool CDispatchEngineSink::OnDBPCExChangeRoomCardToScoreResult(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 提取数据
  DBO_GP_RoomCardExchangeResult* pExchangeResult = (DBO_GP_RoomCardExchangeResult*) pData;

  // 构造结构
  CMD_GP_ExchangeRoomCardResult ExchangeResult;
  ZeroMemory(&ExchangeResult, sizeof(ExchangeResult));

  // 设置变量
  ExchangeResult.bSuccessed = pExchangeResult->bSuccessed;
  ExchangeResult.lCurrScore = pExchangeResult->lCurrScore;
  ExchangeResult.lRoomCard = pExchangeResult->lCurrRoomCard;
  LSTRCPYN(ExchangeResult.szNotifyContent, pExchangeResult->szNotifyContent, std::size(ExchangeResult.szNotifyContent));

  // 发送数据
  WORD wSendDataSize = sizeof(ExchangeResult) - sizeof(ExchangeResult.szNotifyContent);
  wSendDataSize += CountStringBuffer(ExchangeResult.szNotifyContent);
  network_engine_->SendData(dwContextID, MDM_MB_PERSONAL_SERVICE, SUB_GP_EXCHANGE_ROOM_CARD_RESULT, &ExchangeResult, wSendDataSize);

  return true;
}

// 兑换结果
bool CDispatchEngineSink::OnDBPCExChangeResult(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 提取数据
  DBO_GP_ExchangeResult* pExchangeResult = (DBO_GP_ExchangeResult*) pData;

  // 构造结构
  CMD_GP_ExchangeResult ExchangeResult;
  ZeroMemory(&ExchangeResult, sizeof(ExchangeResult));

  // 设置变量
  ExchangeResult.bSuccessed = pExchangeResult->bSuccessed;
  ExchangeResult.lCurrScore = pExchangeResult->lCurrScore;
  ExchangeResult.lCurrIngot = pExchangeResult->lCurrIngot;
  ExchangeResult.dCurrBeans = pExchangeResult->dCurrBeans;
  LSTRCPYN(ExchangeResult.szNotifyContent, pExchangeResult->szNotifyContent, std::size(ExchangeResult.szNotifyContent));

  // 发送数据
  WORD wSendDataSize = sizeof(ExchangeResult) - sizeof(ExchangeResult.szNotifyContent);
  wSendDataSize += CountStringBuffer(ExchangeResult.szNotifyContent);
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_EXCHANGE_RESULT, &ExchangeResult, wSendDataSize);

  return true;
}

// 机器操作
bool CDispatchEngineSink::OnDBAndroidParameter(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_AndroidParameter* pAndroidParameter = (DBO_GP_AndroidParameter*) pData;

  // 变量定义
  CMD_GP_AndroidParameter AndroidParameter;
  ZeroMemory(&AndroidParameter, sizeof(AndroidParameter));

  // 构造变量
  AndroidParameter.wSubCommdID = pAndroidParameter->wSubCommdID;
  AndroidParameter.wParameterCount = pAndroidParameter->wParameterCount;
  CopyMemory(AndroidParameter.AndroidParameter, pAndroidParameter->AndroidParameter, sizeof(tagAndroidParameter) * AndroidParameter.wParameterCount);

  // 计算大小
  WORD wSendDataSize = sizeof(AndroidParameter) - sizeof(AndroidParameter.AndroidParameter);
  wSendDataSize += sizeof(tagAndroidParameter) * AndroidParameter.wParameterCount;

  // 发送数据
  network_engine_->SendData(dwContextID, MDM_GP_ANDROID_SERVICE, SUB_GP_ANDROID_PARAMETER, &AndroidParameter, wSendDataSize);

  // 数量判断
  if (AndroidParameter.wParameterCount == 1) {
    // 协调房间
    if (AndroidParameter.wSubCommdID == SUB_GP_ADD_PARAMETER) {
      // 构造结构
      CMD_CS_C_AddParameter AddParameter;
      AddParameter.wServerID = pAndroidParameter->wServerID;
      CopyMemory(&AddParameter.AndroidParameter, &AndroidParameter.AndroidParameter[0], sizeof(tagAndroidParameter));

      // 发送数据
      correspond_service_->SendData(MDM_CS_ANDROID_SERVICE, SUB_CS_C_ADDPARAMETER, &AddParameter, sizeof(AddParameter));
    }

    // 协调房间
    if (AndroidParameter.wSubCommdID == SUB_GP_MODIFY_PARAMETER) {
      // 构造结构
      CMD_CS_C_ModifyParameter ModifyParameter;
      ModifyParameter.wServerID = pAndroidParameter->wServerID;
      CopyMemory(&ModifyParameter.AndroidParameter, &AndroidParameter.AndroidParameter[0], sizeof(tagAndroidParameter));

      // 发送数据
      correspond_service_->SendData(MDM_CS_ANDROID_SERVICE, SUB_CS_C_MODIFYPARAMETER, &ModifyParameter, sizeof(ModifyParameter));
    }

    // 协调房间
    if (AndroidParameter.wSubCommdID == SUB_GP_DELETE_PARAMETER) {
      // 构造结构
      CMD_CS_C_DeleteParameter DeleteParameter;
      DeleteParameter.wServerID = pAndroidParameter->wServerID;
      DeleteParameter.dwBatchID = AndroidParameter.AndroidParameter[0].dwBatchID;

      // 发送数据
      correspond_service_->SendData(MDM_CS_ANDROID_SERVICE, SUB_CS_C_DELETEPARAMETER, &DeleteParameter, sizeof(DeleteParameter));
    }
  }

  return true;
}

bool CDispatchEngineSink::OnDBIndividualResult(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 提取数据
  DBO_GP_IndividualResult* pResult = (DBO_GP_IndividualResult*) pData;

  // 构造结构
  CMD_GP_IndividuaResult currResult;
  ZeroMemory(&currResult, sizeof(currResult));

  // 设置变量
  currResult.bSuccessed = pResult->bSuccessed;
  currResult.lCurrDiamond = pResult->lDiamond;
  LSTRCPYN(currResult.szNotifyContent, pResult->szDescribeString, std::size(currResult.szNotifyContent));

  // 发送数据
  WORD wSendDataSize = sizeof(currResult) - sizeof(currResult.szNotifyContent);
  wSendDataSize += CountStringBuffer(currResult.szNotifyContent);
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_INDIVIDUAL_RESULT, &currResult, wSendDataSize);

  return true;
}

// 操作成功
bool CDispatchEngineSink::OnDBPCOperateSuccess(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  CMD_GP_OperateSuccess OperateSuccess;
  ZeroMemory(&OperateSuccess, sizeof(OperateSuccess));

  // 变量定义
  DBO_GP_OperateSuccess* pOperateSuccess = (DBO_GP_OperateSuccess*) pData;

  // 构造数据
  OperateSuccess.lResultCode = pOperateSuccess->lResultCode;
  LSTRCPYN(OperateSuccess.szDescribeString, pOperateSuccess->szDescribeString, std::size(OperateSuccess.szDescribeString));

  // 发送数据
  WORD wDescribe = CountStringBuffer(OperateSuccess.szDescribeString);
  WORD wHeadSize = sizeof(OperateSuccess) - sizeof(OperateSuccess.szDescribeString);

  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_OPERATE_SUCCESS, &OperateSuccess, wHeadSize + wDescribe);

  // 关闭连接
  if (pOperateSuccess->bCloseSocket == true) {
    network_engine_->ShutDownSocket(dwContextID);
  }
  return true;
}

// 操作失败
bool CDispatchEngineSink::OnDBPCOperateFailure(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  CMD_GP_OperateFailure OperateFailure;
  ZeroMemory(&OperateFailure, sizeof(OperateFailure));

  // 变量定义
  DBO_GP_OperateFailure* pOperateFailure = (DBO_GP_OperateFailure*) pData;

  // 构造数据
  OperateFailure.lResultCode = pOperateFailure->lResultCode;
  LSTRCPYN(OperateFailure.szDescribeString, pOperateFailure->szDescribeString, std::size(OperateFailure.szDescribeString));

  // 发送数据
  WORD wDescribe = CountStringBuffer(OperateFailure.szDescribeString);
  WORD wHeadSize = sizeof(OperateFailure) - sizeof(OperateFailure.szDescribeString);
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_OPERATE_FAILURE, &OperateFailure, wHeadSize + wDescribe);

  // 关闭连接
  if (pOperateFailure->bCloseSocket == true) {
    network_engine_->ShutDownSocket(dwContextID);
  }
  return true;
}

// 登录成功
bool CDispatchEngineSink::OnDBMBLogonSuccess(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  BYTE cbDataBuffer[SOCKET_TCP_PACKET];
  DBO_MB_LogonSuccess* pDBOLogonSuccess = (DBO_MB_LogonSuccess*) pData;
  CMD_MB_LogonSuccess* pCMDLogonSuccess = (CMD_MB_LogonSuccess*) cbDataBuffer;

  // 发送定义
  WORD wHeadSize = sizeof(CMD_MB_LogonSuccess);
  CSendPacketHelper SendPacket(cbDataBuffer + wHeadSize, sizeof(cbDataBuffer) - wHeadSize);

  // 设置变量
  ZeroMemory(pCMDLogonSuccess, sizeof(CMD_MB_LogonSuccess));

  // 构造数据
  pCMDLogonSuccess->wFaceID = pDBOLogonSuccess->wFaceID;
  pCMDLogonSuccess->cbGender = pDBOLogonSuccess->cbGender;
  pCMDLogonSuccess->dwCustomID = pDBOLogonSuccess->dwCustomID;
  pCMDLogonSuccess->dwGameID = pDBOLogonSuccess->dwGameID;
  pCMDLogonSuccess->dwUserID = pDBOLogonSuccess->dwUserID;
  //	pCMDLogonSuccess->dwExperience=pDBOLogonSuccess->dwExperience;
  //	pCMDLogonSuccess->lLoveLiness=pDBOLogonSuccess->lLoveLiness;
  LSTRCPYN(pCMDLogonSuccess->szAccounts, pDBOLogonSuccess->szAccounts, std::size(pCMDLogonSuccess->szAccounts));
  LSTRCPYN(pCMDLogonSuccess->szNickName, pDBOLogonSuccess->szNickName, std::size(pCMDLogonSuccess->szNickName));
  LSTRCPYN(pCMDLogonSuccess->szDynamicPass, pDBOLogonSuccess->szDynamicPass, std::size(pCMDLogonSuccess->szDynamicPass));

  // 用户成绩
  pCMDLogonSuccess->lUserScore = pDBOLogonSuccess->lUserScore;
  //	pCMDLogonSuccess->lUserIngot=pDBOLogonSuccess->lUserIngot;
  pCMDLogonSuccess->lUserInsure = pDBOLogonSuccess->lUserInsure;
  //	pCMDLogonSuccess->dUserBeans=pDBOLogonSuccess->dUserBeans;
  pCMDLogonSuccess->lDiamond = pDBOLogonSuccess->lDiamond;

  // 扩展信息
  pCMDLogonSuccess->cbInsureEnabled = pDBOLogonSuccess->cbInsureEnabled;
  pCMDLogonSuccess->cbIsAgent = pDBOLogonSuccess->cbIsAgent;
  pCMDLogonSuccess->cbMoorMachine = pDBOLogonSuccess->cbMoorMachine;
  // pCMDLogonSuccess->lRoomCard = pDBOLogonSuccess->lRoomCard;
  pCMDLogonSuccess->dwLockServerID = pDBOLogonSuccess->dwLockServerID;
  pCMDLogonSuccess->dwKindID = pDBOLogonSuccess->dwKindID;

  // 会员信息
  if (pDBOLogonSuccess->cbMemberOrder != 0) {
    DTP_GP_MemberInfo MemberInfo;
    ZeroMemory(&MemberInfo, sizeof(MemberInfo));
    MemberInfo.cbMemberOrder = pDBOLogonSuccess->cbMemberOrder;
    MemberInfo.MemberOverDate = pDBOLogonSuccess->MemberOverDate;
    SendPacket.AddPacket(&MemberInfo, sizeof(MemberInfo), DTP_GP_MEMBER_INFO);
  }

  // 个性签名
  if (pDBOLogonSuccess->szUnderWrite[0] != 0) {
    SendPacket.AddPacket(pDBOLogonSuccess->szUnderWrite, CountStringBuffer(pDBOLogonSuccess->szUnderWrite), DTP_GP_UNDER_WRITE);
  }

  // 登录成功
  WORD wSendSize = SendPacket.GetDataSize() + sizeof(CMD_MB_LogonSuccess);
  network_engine_->SendData(dwContextID, MDM_MB_LOGON, SUB_MB_LOGON_SUCCESS, cbDataBuffer, wSendSize);

  // 会员配置
  //	SendMemberConfig(dwContextID);

  // 发送房间
  WORD wIndex = LOWORD(dwContextID);
  SendMobileKindInfo(dwContextID, (bind_parameter_ + wIndex)->wModuleID);
  SendMobileServerInfo(dwContextID, (bind_parameter_ + wIndex)->wModuleID);
  network_engine_->SendData(dwContextID, MDM_MB_SERVER_LIST, SUB_MB_LIST_FINISH);

  return true;
}

// 登录失败
bool CDispatchEngineSink::OnDBMBLogonFailure(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  CMD_MB_LogonFailure LogonFailure;
  ZeroMemory(&LogonFailure, sizeof(LogonFailure));
  DBO_MB_LogonFailure* pLogonFailure = (DBO_MB_LogonFailure*) pData;

  // 构造数据
  LogonFailure.lResultCode = pLogonFailure->lResultCode;
  LSTRCPYN(LogonFailure.szDescribeString, pLogonFailure->szDescribeString, std::size(LogonFailure.szDescribeString));

  // 发送数据
  WORD wStringSize = CountStringBuffer(LogonFailure.szDescribeString);
  WORD wSendSize = sizeof(LogonFailure) - sizeof(LogonFailure.szDescribeString) + wStringSize;
  network_engine_->SendData(dwContextID, MDM_MB_LOGON, SUB_MB_LOGON_FAILURE, &LogonFailure, wSendSize);

  // 关闭连接
  network_engine_->ShutDownSocket(dwContextID);

  return true;
}

// 抽奖配置
bool CDispatchEngineSink::OnDBPCLotteryConfig(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_LotteryConfig* pLotteryConfig = (DBO_GP_LotteryConfig*) pData;

  // 变量定义
  CMD_GP_LotteryConfig LotteryConfig;
  ZeroMemory(&LotteryConfig, sizeof(LotteryConfig));

  // 构造变量
  LotteryConfig.wLotteryCount = pLotteryConfig->wLotteryCount;
  CopyMemory(LotteryConfig.LotteryItem, pLotteryConfig->LotteryItem, sizeof(LotteryConfig.LotteryItem));

  // 发送数据
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_LOTTERY_CONFIG, &LotteryConfig, sizeof(LotteryConfig));

  return true;
}

// 抽奖信息
bool CDispatchEngineSink::OnDBPCLotteryUserInfo(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_LotteryUserInfo* pLotteryConfig = (DBO_GP_LotteryUserInfo*) pData;

  // 变量定义
  CMD_GP_LotteryUserInfo LotteryUserInfo;
  ZeroMemory(&LotteryUserInfo, sizeof(LotteryUserInfo));

  // 构造变量
  LotteryUserInfo.cbFreeCount = pLotteryConfig->cbFreeCount;
  LotteryUserInfo.cbAlreadyCount = pLotteryConfig->cbAlreadyCount;
  LotteryUserInfo.wKindID = pLotteryConfig->wKindID;
  LotteryUserInfo.dwUserID = pLotteryConfig->dwUserID;
  LotteryUserInfo.lChargeFee = pLotteryConfig->lChargeFee;

  // 发送数据
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_LOTTERY_USER_INFO, &LotteryUserInfo, sizeof(LotteryUserInfo));

  return true;
}

// 抽奖结果
bool CDispatchEngineSink::OnDBPCLotteryResult(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  DBO_GP_LotteryResult* pLotteryResult = (DBO_GP_LotteryResult*) pData;

  // 变量定义
  CMD_GP_LotteryResult LotteryResult;
  ZeroMemory(&LotteryResult, sizeof(LotteryResult));

  // 构造变量
  LotteryResult.bWined = pLotteryResult->bWined;
  LotteryResult.wKindID = pLotteryResult->wKindID;
  LotteryResult.dwUserID = pLotteryResult->dwUserID;
  LotteryResult.lUserScore = pLotteryResult->lUserScore;
  LotteryResult.dUserBeans = pLotteryResult->dUserBeans;
  CopyMemory(&LotteryResult.LotteryItem, &pLotteryResult->LotteryItem, sizeof(LotteryResult.LotteryItem));

  // 发送数据
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_LOTTERY_RESULT, &LotteryResult, sizeof(LotteryResult));

  return true;
}

// 游戏数据
bool CDispatchEngineSink::OnDBPCQueryUserGameData(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 判断在线
  ASSERT(LOWORD(dwContextID) < init_parameter_->max_connect_);
  if ((bind_parameter_ + LOWORD(dwContextID))->dwSocketID != dwContextID)
    return true;

  // 变量定义
  BYTE cbDataBuffer[SOCKET_TCP_PACKET];
  DBO_GP_QueryUserGameData* pDBOQueryUserGameData = (DBO_GP_QueryUserGameData*) pData;
  CMD_GP_QueryUserGameData* pCMDQueryUserGameData = (CMD_GP_QueryUserGameData*) cbDataBuffer;
  CSendPacketHelper SendPacket(cbDataBuffer + sizeof(CMD_GP_QueryUserGameData), sizeof(cbDataBuffer) - sizeof(CMD_GP_QueryUserGameData));

  // 设置变量
  ZeroMemory(pCMDQueryUserGameData, sizeof(CMD_GP_QueryUserGameData));

  // 构造变量
  pCMDQueryUserGameData->wKindID = pDBOQueryUserGameData->wKindID;
  pCMDQueryUserGameData->dwUserID = pDBOQueryUserGameData->dwUserID;

  // 联系地址
  if (pDBOQueryUserGameData->szUserGameData[0] != 0) {
    WORD wBufferSize = CountStringBuffer(pDBOQueryUserGameData->szUserGameData);
    SendPacket.AddPacket(pDBOQueryUserGameData->szUserGameData, wBufferSize, DTP_GP_UI_USER_GAME_DATA);
  }

  // 发送消息
  WORD wSendSize = sizeof(CMD_GP_QueryUserGameData) + SendPacket.GetDataSize();
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_QUERY_USER_GAME_DATA, cbDataBuffer, wSendSize);

  return true;
}

// 代理列表
bool CDispatchEngineSink::OnDBPCAgentGameList(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize % sizeof(tagAgentGameKind) == 0);
  if (wDataSize % sizeof(tagAgentGameKind) != 0)
    return false;
  network_engine_->SendData(dwContextID, MDM_GP_SERVER_LIST, SUB_GP_AGENT_KIND, pData, wDataSize);

  return true;
}

// 代理列表
bool CDispatchEngineSink::OnDBMBAgentGameList(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize % sizeof(tagAgentGameKind) == 0);
  if (wDataSize % sizeof(tagAgentGameKind) != 0)
    return false;
  network_engine_->SendData(dwContextID, MDM_MB_SERVER_LIST, SUB_MB_AGENT_KIND, pData, wDataSize);

  return true;
}

// 约战房间配置
bool CDispatchEngineSink::OnDBMBPersonalParameter(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize % sizeof(tagPersonalRoomOption) == 0);
  if (wDataSize % sizeof(tagPersonalRoomOption) != 0)
    return false;

  network_engine_->SendData(dwContextID, MDM_MB_PERSONAL_SERVICE, SUB_MB_PERSONAL_PARAMETER, pData, wDataSize);

  return true;
}

// 约战房间配置
bool CDispatchEngineSink::OnDBMBPersonalFeeList(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize % sizeof(tagPersonalTableFeeList) == 0);
  if (wDataSize % sizeof(tagPersonalTableFeeList) != 0)
    return false;

  network_engine_->SendData(dwContextID, MDM_MB_PERSONAL_SERVICE, SUB_MB_PERSONAL_FEE_PARAMETER, pData, wDataSize);

  return true;
}
// 私人房间配置
bool CDispatchEngineSink::OnDBMBPersonalCellScore(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize % sizeof(tagPersonalCellScore) == 0);
  if (wDataSize % sizeof(tagPersonalCellScore) != 0)
    return false;

  network_engine_->SendData(dwContextID, MDM_MB_PERSONAL_SERVICE, SUB_MB_PERSONAL_CELL_SCORE, pData, wDataSize);

  return true;
}
// 私人房间定制配置
bool CDispatchEngineSink::OnDBMBPersonalRule(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize == sizeof(tagGetPersonalRule));
  if (wDataSize != sizeof(tagGetPersonalRule))
    return false;

  network_engine_->SendData(dwContextID, MDM_MB_PERSONAL_SERVICE, SUB_MB_PERSONAL_RULE_RESULT, pData, wDataSize);

  return true;
}
// 请求约战房间列表
bool CDispatchEngineSink::OnDBMBPersonalRoomListInfo(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize % sizeof(DBO_MB_PersonalRoomInfoList) == 0);
  if (wDataSize % sizeof(DBO_MB_PersonalRoomInfoList) != 0)
    return false;
  DBO_MB_PersonalRoomInfoList* pDBOPersonalRoomInfoList = (DBO_MB_PersonalRoomInfoList*) pData;
  CMD_MB_PersonalRoomInfoList PersonalRoomInfoList;
  memcpy(&PersonalRoomInfoList, pDBOPersonalRoomInfoList, sizeof(PersonalRoomInfoList));

  network_engine_->SendData(dwContextID, MDM_MB_PERSONAL_SERVICE, SUB_MB_QUERY_PERSONAL_ROOM_LIST_RESULT, &PersonalRoomInfoList,
                            sizeof(CMD_MB_PersonalRoomInfoList));

  return true;
}

// 约战房间玩家请求房间信息
bool CDispatchEngineSink::OnDBQueryUserRoomScore(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  if (wDataSize == 0) {
    network_engine_->SendData(dwContextID, MDM_MB_PERSONAL_SERVICE, SUB_GR_USER_QUERY_ROOM_SCORE_RESULT, nullptr, 0);
    return true;
  }
  // 效验参数
  ASSERT(wDataSize % sizeof(tagQueryPersonalRoomUserScore) == 0);
  if (wDataSize % sizeof(tagQueryPersonalRoomUserScore) != 0)
    return false;

  network_engine_->SendData(dwContextID, MDM_MB_PERSONAL_SERVICE, SUB_GR_USER_QUERY_ROOM_SCORE_RESULT, pData, wDataSize);

  return true;
}

// 约战房间玩家请求房间信息
bool CDispatchEngineSink::OnDBQueryPersonalRoomUersInfoResult(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize == sizeof(DBO_MB_PersonalRoomUserInfo));
  if (wDataSize != sizeof(DBO_MB_PersonalRoomUserInfo))
    return false;
  DBO_MB_PersonalRoomUserInfo* pUserInfo = (DBO_MB_PersonalRoomUserInfo*) pData;
  CMD_MB_PersonalRoomUserInfo userInfo;
  // userInfo.dBeans = pUserInfo->dBeans;
  userInfo.lDiamond = pUserInfo->lDiamond;

  network_engine_->SendData(dwContextID, MDM_MB_PERSONAL_SERVICE, SUB_MB_QUERY_PERSONAL_ROOM_USER_INFO_RESULT, &userInfo, sizeof(userInfo));

  return true;
}

// 游戏种类
bool CDispatchEngineSink::OnDBPCGameTypeItem(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize % sizeof(DBO_GP_GameType) == 0);
  if (wDataSize % sizeof(DBO_GP_GameType) != 0)
    return false;

  // 变量定义
  WORD wItemCount = wDataSize / sizeof(DBO_GP_GameType);
  DBO_GP_GameType* pGameType = (DBO_GP_GameType*) pData;

  // 更新数据
  for (WORD i = 0; i < wItemCount; i++) {
    // 变量定义
    tagGameType GameType;
    ZeroMemory(&GameType, sizeof(GameType));

    // 构造数据
    GameType.wTypeID = (pGameType + i)->wTypeID;
    GameType.wJoinID = (pGameType + i)->wJoinID;
    GameType.wSortID = (pGameType + i)->wSortID;
    LSTRCPYN(GameType.szTypeName, (pGameType + i)->szTypeName, std::size(GameType.szTypeName));

    // 插入列表
    m_ServerListManager.InsertGameType(&GameType);
  }

  return true;
}

// 游戏类型
bool CDispatchEngineSink::OnDBPCGameKindItem(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize % sizeof(DBO_GP_GameKind) == 0);
  if (wDataSize % sizeof(DBO_GP_GameKind) != 0)
    return false;

  // 变量定义
  WORD wItemCount = wDataSize / sizeof(DBO_GP_GameKind);
  DBO_GP_GameKind* pGameKind = (DBO_GP_GameKind*) pData;

  // 更新数据
  for (WORD i = 0; i < wItemCount; i++) {
    // 变量定义
    tagGameKind GameKind;
    ZeroMemory(&GameKind, sizeof(GameKind));

    // 构造数据
    GameKind.wTypeID = (pGameKind + i)->wTypeID;
    GameKind.wJoinID = (pGameKind + i)->wJoinID;
    GameKind.wSortID = (pGameKind + i)->wSortID;
    GameKind.wKindID = (pGameKind + i)->wKindID;
    GameKind.wGameID = (pGameKind + i)->wGameID;
    GameKind.wRecommend = (pGameKind + i)->wRecommend;
    GameKind.wGameFlag = (pGameKind + i)->wGameFlag;
    GameKind.dwOnLineCount = m_ServerListManager.CollectOnlineInfo((pGameKind + i)->wKindID, false);
    GameKind.dwAndroidCount = m_ServerListManager.CollectOnlineInfo((pGameKind + i)->wKindID, true);
    GameKind.dwSetCount = m_ServerListManager.CollectSetPlayer((pGameKind + i)->wKindID);
    LSTRCPYN(GameKind.szKindName, (pGameKind + i)->szKindName, std::size(GameKind.szKindName));
    LSTRCPYN(GameKind.szProcessName, (pGameKind + i)->szProcessName, std::size(GameKind.szProcessName));

    // 插入列表
    m_ServerListManager.InsertGameKind(&GameKind);
  }

  return true;
}

// 游戏节点
bool CDispatchEngineSink::OnDBPCGameNodeItem(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize % sizeof(DBO_GP_GameNode) == 0);
  if (wDataSize % sizeof(DBO_GP_GameNode) != 0)
    return false;

  // 变量定义
  WORD wItemCount = wDataSize / sizeof(DBO_GP_GameNode);
  DBO_GP_GameNode* pGameNode = (DBO_GP_GameNode*) pData;

  // 更新数据
  for (WORD i = 0; i < wItemCount; i++) {
    // 变量定义
    tagGameNode GameNode;
    ZeroMemory(&GameNode, sizeof(GameNode));

    // 构造数据
    GameNode.wKindID = (pGameNode + i)->wKindID;
    GameNode.wJoinID = (pGameNode + i)->wJoinID;
    GameNode.wSortID = (pGameNode + i)->wSortID;
    GameNode.wNodeID = (pGameNode + i)->wNodeID;
    LSTRCPYN(GameNode.szNodeName, (pGameNode + i)->szNodeName, std::size(GameNode.szNodeName));

    // 插入列表
    m_ServerListManager.InsertGameNode(&GameNode);
  }

  return true;
}

// 游戏定制
bool CDispatchEngineSink::OnDBPCGamePageItem(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize % sizeof(DBO_GP_GamePage) == 0);
  if (wDataSize % sizeof(DBO_GP_GamePage) != 0)
    return false;

  // 变量定义
  WORD wItemCount = wDataSize / sizeof(DBO_GP_GamePage);
  DBO_GP_GamePage* pGamePage = (DBO_GP_GamePage*) pData;

  // 更新数据
  for (WORD i = 0; i < wItemCount; i++) {
    // 变量定义
    tagGamePage GamePage;
    ZeroMemory(&GamePage, sizeof(GamePage));

    // 构造数据
    GamePage.wKindID = (pGamePage + i)->wKindID;
    GamePage.wNodeID = (pGamePage + i)->wNodeID;
    GamePage.wSortID = (pGamePage + i)->wSortID;
    GamePage.wPageID = (pGamePage + i)->wPageID;
    GamePage.wOperateType = (pGamePage + i)->wOperateType;
    LSTRCPYN(GamePage.szDisplayName, (pGamePage + i)->szDisplayName, std::size(GamePage.szDisplayName));

    // 插入列表
    m_ServerListManager.InsertGamePage(&GamePage);
  }

  return true;
}

// 游戏列表
bool CDispatchEngineSink::OnDBPCGameListResult(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize == sizeof(DBO_GP_GameListResult));
  if (wDataSize != sizeof(DBO_GP_GameListResult))
    return false;

  // 变量定义
  DBO_GP_GameListResult* pGameListResult = (DBO_GP_GameListResult*) pData;

  // 消息处理
  if (pGameListResult->cbSuccess == TRUE) {
    // 清理列表
    m_ServerListManager.CleanKernelItem();

    // 事件通知
    service_units->OnLoadDbListResult(ER_SUCCESS);

    // 设置时间
    ASSERT(timer_engine_ != nullptr);
    timer_engine_->SetTimer(IDI_LOAD_GAME_LIST, init_parameter_->load_list_time_ * 1000L, 1, 0);
  } else {
    // 构造提示
    CLogger::Warn(TEXT("服务器列表加载失败，{} 秒后将重新加载"), init_parameter_->m_wReLoadListTime);

    // 设置时间
    ASSERT(timer_engine_ != nullptr);
    timer_engine_->SetTimer(IDI_LOAD_GAME_LIST, init_parameter_->m_wReLoadListTime * 1000L, 1, 0);
  }

  return true;
}

// 版本检测
bool CDispatchEngineSink::CheckPlazaVersion(BYTE cbDeviceType, DWORD dwPlazaVersion, DWORD dwSocketID, bool bCheckLowVer) {
  // 变量定义
  bool bMustUpdate = false;
  bool bAdviceUpdate = false;
  DWORD dwVersion = VERSION_PLAZA;

  // 手机版本
  if (cbDeviceType >= DEVICE_TYPE_IPAD)
    dwVersion = VERSION_MOBILE_IOS;
  else if (cbDeviceType >= DEVICE_TYPE_IPHONE)
    dwVersion = VERSION_MOBILE_IOS;
  else if (cbDeviceType >= DEVICE_TYPE_ITOUCH)
    dwVersion = VERSION_MOBILE_IOS;
  else if (cbDeviceType >= DEVICE_TYPE_ANDROID)
    dwVersion = VERSION_MOBILE_ANDROID;
  else if (cbDeviceType == DEVICE_TYPE_PC)
    dwVersion = VERSION_PLAZA;

  // 版本判断
  if (bCheckLowVer && GetSubVer(dwPlazaVersion) < GetSubVer(dwVersion))
    bAdviceUpdate = true;
  if (GetMainVer(dwPlazaVersion) != GetMainVer(dwVersion))
    bMustUpdate = true;
  if (GetProductVer(dwPlazaVersion) != GetProductVer(dwVersion))
    bMustUpdate = true;

  // 升级判断
  if ((bMustUpdate == true) || (bAdviceUpdate == true)) {
    // 变量定义
    CMD_GP_UpdateNotify UpdateNotify;
    ZeroMemory(&UpdateNotify, sizeof(UpdateNotify));

    // 变量定义
    UpdateNotify.cbMustUpdate = bMustUpdate;
    UpdateNotify.cbAdviceUpdate = bAdviceUpdate;
    UpdateNotify.dwCurrentVersion = dwVersion;

    // 发送消息
    network_engine_->SendData(dwSocketID, MDM_GP_LOGON, SUB_GP_UPDATE_NOTIFY, &UpdateNotify, sizeof(UpdateNotify));

    // 中断判断
    if (bMustUpdate == true) {
      network_engine_->ShutDownSocket(dwSocketID);
      return false;
    }
  }

  return true;
}

// 发送类型
VOID CDispatchEngineSink::SendGameTypeInfo(DWORD dwSocketID) {
  // 网络数据
  WORD wSendSize = 0;
  BYTE cbDataBuffer[SOCKET_TCP_PACKET];

  // 枚举数据
  CTypeItemMap::iterator* Position = nullptr;
  CGameTypeItem* pGameTypeItem = nullptr;

  // 枚举数据
  for (DWORD i = 0; i < m_ServerListManager.GetGameTypeCount(); i++) {
    // 发送数据
    if ((wSendSize + sizeof(tagGameType)) > sizeof(cbDataBuffer)) {
      network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_TYPE, cbDataBuffer, wSendSize);
      wSendSize = 0;
    }

    // 获取数据
    pGameTypeItem = m_ServerListManager.EmunGameTypeItem(Position);
    if (pGameTypeItem == nullptr)
      break;

    // 拷贝数据
    CopyMemory(cbDataBuffer + wSendSize, &pGameTypeItem->m_GameType, sizeof(tagGameType));
    wSendSize += sizeof(tagGameType);
  }

  // 发送剩余
  if (wSendSize > 0)
    network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_TYPE, cbDataBuffer, wSendSize);
}

// 发送种类
VOID CDispatchEngineSink::SendGameKindInfo(DWORD dwSocketID) {
  // 网络数据
  WORD wSendSize = 0;
  BYTE cbDataBuffer[SOCKET_TCP_PACKET];

  // 枚举数据
  CKindItemMap::iterator* Position = nullptr;
  CGameKindItem* pGameKindItem = nullptr;

  // 枚举数据
  for (DWORD i = 0; i < m_ServerListManager.GetGameKindCount(); i++) {
    // 发送数据
    if ((wSendSize + sizeof(tagGameKind)) > sizeof(cbDataBuffer)) {
      network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_KIND, cbDataBuffer, wSendSize);
      wSendSize = 0;
    }

    // 获取数据
    pGameKindItem = m_ServerListManager.EmunGameKindItem(Position);
    if (pGameKindItem == nullptr)
      break;

    // 拷贝数据
    CopyMemory(cbDataBuffer + wSendSize, &pGameKindItem->m_GameKind, sizeof(tagGameKind));
    wSendSize += sizeof(tagGameKind);
  }

  // 发送剩余
  if (wSendSize > 0)
    network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_KIND, cbDataBuffer, wSendSize);
}

// 发送节点
VOID CDispatchEngineSink::SendGameNodeInfo(DWORD dwSocketID, WORD wKindID) {
  // 网络数据
  WORD wSendSize = 0;
  BYTE cbDataBuffer[SOCKET_TCP_PACKET];

  // 枚举数据
  CNodeItemMap::iterator* Position = nullptr;
  CGameNodeItem* pGameNodeItem = nullptr;

  // 枚举数据
  for (DWORD i = 0; i < m_ServerListManager.GetGameNodeCount(); i++) {
    // 发送数据
    if ((wSendSize + sizeof(tagGameNode)) > sizeof(cbDataBuffer)) {
      network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_NODE, cbDataBuffer, wSendSize);
      wSendSize = 0;
    }

    // 获取数据
    pGameNodeItem = m_ServerListManager.EmunGameNodeItem(Position);
    if (pGameNodeItem == nullptr)
      break;

    // 拷贝数据
    if ((wKindID == INVALID_WORD) || (pGameNodeItem->m_GameNode.wKindID == wKindID)) {
      CopyMemory(cbDataBuffer + wSendSize, &pGameNodeItem->m_GameNode, sizeof(tagGameNode));
      wSendSize += sizeof(tagGameNode);
    }
  }

  // 发送剩余
  if (wSendSize > 0)
    network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_NODE, cbDataBuffer, wSendSize);
}

// 发送定制
VOID CDispatchEngineSink::SendGamePageInfo(DWORD dwSocketID, WORD wKindID) {
  // 网络数据
  WORD wSendSize = 0;
  BYTE cbDataBuffer[SOCKET_TCP_PACKET];

  // 枚举数据
  CPageItemMap::iterator* Position = nullptr;
  CGamePageItem* pGamePageItem = nullptr;

  // 枚举数据
  for (DWORD i = 0; i < m_ServerListManager.GetGamePageCount(); i++) {
    // 发送数据
    if ((wSendSize + sizeof(tagGamePage)) > sizeof(cbDataBuffer)) {
      network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_PAGE, cbDataBuffer, wSendSize);
      wSendSize = 0;
    }

    // 获取数据
    pGamePageItem = m_ServerListManager.EmunGamePageItem(Position);
    if (pGamePageItem == nullptr)
      break;

    // 拷贝数据
    if ((wKindID == INVALID_WORD) || (pGamePageItem->m_GamePage.wKindID == wKindID)) {
      CopyMemory(cbDataBuffer + wSendSize, &pGamePageItem->m_GamePage, sizeof(tagGamePage));
      wSendSize += sizeof(tagGamePage);
    }
  }

  // 发送剩余
  if (wSendSize > 0)
    network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_PAGE, cbDataBuffer, wSendSize);
}

// 发送房间
VOID CDispatchEngineSink::SendGameServerInfo(DWORD dwSocketID, WORD wKindID, BYTE cbDeviceType) {
  // 网络数据
  WORD wSendSize = 0;
  BYTE cbDataBuffer[SOCKET_TCP_PACKET];

  // 枚举数据
  CServerItemMap::iterator* Position = nullptr;
  CGameServerItem* pGameServerItem = nullptr;

  // 枚举数据
  for (DWORD i = 0; i < m_ServerListManager.GetGameServerCount(); i++) {
    // 发送数据
    if ((wSendSize + sizeof(tagGameServer)) > sizeof(cbDataBuffer)) {
      network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_SERVER, cbDataBuffer, wSendSize);
      wSendSize = 0;
    }

    // 获取数据
    pGameServerItem = m_ServerListManager.EmunGameServerItem(Position);
    if (pGameServerItem == nullptr)
      break;
    // 支持类型
    bool bSupportMobile = CServerRule::IsSupportMobile(pGameServerItem->m_GameServer.dwServerRule);
    bool bSupportPC = CServerRule::IsSupportPC(pGameServerItem->m_GameServer.dwServerRule);

    if (cbDeviceType == DEVICE_TYPE_PC) {
      if (bSupportPC == false)
        continue;
    } else if (cbDeviceType != DEVICE_TYPE_PC) {
      if (bSupportMobile == false)
        continue;
    }

    // 拷贝数据
    if ((wKindID == INVALID_WORD) || (pGameServerItem->m_GameServer.wKindID == wKindID)) {
      CopyMemory(cbDataBuffer + wSendSize, &pGameServerItem->m_GameServer, sizeof(tagGameServer));
      wSendSize += sizeof(tagGameServer);
    }
  }

  // 发送剩余
  if (wSendSize > 0)
    network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_SERVER, cbDataBuffer, wSendSize);
  // 设置变量
  wSendSize = 0;
  ZeroMemory(cbDataBuffer, sizeof(cbDataBuffer));

  // 枚举数据
  for (DWORD i = 0; i < m_ServerListManager.GetGameServerCount(); i++) {
    // 发送数据
    if ((wSendSize + sizeof(tagGameMatch)) > sizeof(cbDataBuffer)) {
      network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_MATCH, cbDataBuffer, wSendSize);
      wSendSize = 0;
    }

    // 获取数据
    pGameServerItem = m_ServerListManager.EmunGameServerItem(Position);
    if (pGameServerItem == nullptr)
      break;
    if (pGameServerItem->IsMatchServer() == false)
      continue;

    // 拷贝数据
    if ((wKindID == INVALID_WORD) || (pGameServerItem->m_GameServer.wKindID == wKindID)) {
      CopyMemory(cbDataBuffer + wSendSize, &pGameServerItem->m_GameMatch, sizeof(tagGameMatch));
      wSendSize += sizeof(tagGameMatch);
    }
  }

  // 发送剩余
  if (wSendSize > 0)
    network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_MATCH, cbDataBuffer, wSendSize);

  if (m_wAVServerPort != 0 && m_dwAVServerAddr != 0) {
    // 变量定义
    tagAVServerOption AVServerOption;
    AVServerOption.wAVServerPort = m_wAVServerPort;
    AVServerOption.dwAVServerAddr = m_dwAVServerAddr;

    // 发送配置
    network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_VIDEO_OPTION, &AVServerOption, sizeof(AVServerOption));
  };
}

// 等级配置
VOID CDispatchEngineSink::SendGrowLevelConfig(DWORD dwSocketID) {
  // 构造结构
  CMD_GP_GrowLevelConfig GrowLevelConfig;
  GrowLevelConfig.wLevelCount = m_wLevelCount;
  CopyMemory(GrowLevelConfig.GrowLevelItem, m_GrowLevelConfig, sizeof(tagGrowLevelConfig) * GrowLevelConfig.wLevelCount);

  // 发送数据
  WORD wDataSize = sizeof(GrowLevelConfig) - sizeof(GrowLevelConfig.GrowLevelItem);
  wDataSize += sizeof(tagGrowLevelConfig) * GrowLevelConfig.wLevelCount;
  network_engine_->SendData(dwSocketID, MDM_GP_LOGON, SUB_GP_GROWLEVEL_CONFIG, &GrowLevelConfig, wDataSize);
}

// 道具类型
VOID CDispatchEngineSink::SendGamePropertyTypeInfo(DWORD dwSocketID) {
  // 网络数据
  WORD wSendSize = 0;
  BYTE cbDataBuffer[SOCKET_TCP_PACKET];

  // 枚举数据
  CGamePropertyTypeItemMap::iterator* Position = nullptr;
  CGamePropertyTypeItem* pGamePropertyTypeItem = nullptr;
  DWORD dwCount = m_GamePropertyListManager.GetGamePropertyTypeCount();

  // 枚举数据
  for (DWORD i = 0; i < dwCount; i++) {
    // 发送数据
    if ((wSendSize + sizeof(tagPropertyTypeItem)) > sizeof(cbDataBuffer)) {
      network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_PROPERTY_TYPE, cbDataBuffer, wSendSize);
      wSendSize = 0;
    }

    // 获取数据
    pGamePropertyTypeItem = m_GamePropertyListManager.EmunGamePropertyTypeItem(Position);
    if (pGamePropertyTypeItem == nullptr)
      break;

    // 拷贝数据
    CopyMemory(cbDataBuffer + wSendSize, &pGamePropertyTypeItem->m_PropertyTypeItem, sizeof(tagPropertyTypeItem));
    wSendSize += sizeof(tagPropertyTypeItem);
  }

  // 发送剩余
  if (wSendSize > 0)
    network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_PROPERTY_TYPE, cbDataBuffer, wSendSize);
}

// 道具关系
VOID CDispatchEngineSink::SendGamePropertyRelatInfo(DWORD dwSocketID) {
  // 网络数据
  WORD wSendSize = 0;
  BYTE cbDataBuffer[SOCKET_TCP_PACKET];

  // 枚举数据
  CGamePropertyRelatItemMap::iterator* Position = nullptr;
  CGamePropertyRelatItem* pGamePropertyRelatItem = nullptr;
  DWORD dwCount = m_GamePropertyListManager.GetGamePropertyRelatCount();

  // 枚举数据
  for (DWORD i = 0; i < dwCount; i++) {
    // 发送数据
    if ((wSendSize + sizeof(tagPropertyRelatItem)) > sizeof(cbDataBuffer)) {
      network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_PROPERTY_RELAT, cbDataBuffer, wSendSize);
      wSendSize = 0;
    }

    // 获取数据
    pGamePropertyRelatItem = m_GamePropertyListManager.EmunGamePropertyRelatItem(Position);
    if (pGamePropertyRelatItem == nullptr)
      break;

    // 拷贝数据
    CopyMemory(cbDataBuffer + wSendSize, &pGamePropertyRelatItem->m_PropertyRelatItem, sizeof(tagPropertyRelatItem));
    wSendSize += sizeof(tagPropertyRelatItem);
  }

  // 发送剩余
  if (wSendSize > 0)
    network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_PROPERTY_RELAT, cbDataBuffer, wSendSize);
}

// 发送道具
VOID CDispatchEngineSink::SendGamePropertyInfo(DWORD dwSocketID) {
  // 网络数据
  WORD wSendSize = 0;
  BYTE cbDataBuffer[SOCKET_TCP_PACKET];

  // 枚举数据
  CGamePropertyItemMap::iterator* Position = nullptr;
  CGamePropertyItem* pGamePropertyItem = nullptr;
  DWORD dwCount = m_GamePropertyListManager.GetGamePropertyCount();

  // 枚举数据
  for (DWORD i = 0; i < dwCount; i++) {
    // 发送数据
    if ((wSendSize + sizeof(tagPropertyItem)) > sizeof(cbDataBuffer)) {
      network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_PROPERTY, cbDataBuffer, wSendSize);
      wSendSize = 0;
    }

    // 获取数据
    pGamePropertyItem = m_GamePropertyListManager.EmunGamePropertyItem(Position);
    if (pGamePropertyItem == nullptr)
      break;

    // 拷贝数据
    CopyMemory(cbDataBuffer + wSendSize, &pGamePropertyItem->m_PropertyItem, sizeof(tagPropertyItem));
    wSendSize += sizeof(tagPropertyItem);
  }

  // 发送剩余
  if (wSendSize > 0)
    network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_PROPERTY, cbDataBuffer, wSendSize);
}

// 发送道具
VOID CDispatchEngineSink::SendGamePropertySubInfo(DWORD dwSocketID) {
  // 网络数据
  WORD wSendSize = 0;
  BYTE cbDataBuffer[SOCKET_TCP_PACKET];

  // 枚举数据
  CGamePropertySubItemMap::iterator* Position = nullptr;
  CGamePropertySubItem* pGamePropertySubItem = nullptr;
  DWORD dwCount = m_GamePropertyListManager.GetGamePropertySubCount();

  // 枚举数据
  for (DWORD i = 0; i < dwCount; i++) {
    // 发送数据
    if ((wSendSize + sizeof(CGamePropertySubItem)) > sizeof(cbDataBuffer)) {
      network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_PROPERTY_SUB, cbDataBuffer, wSendSize);
      wSendSize = 0;
    }

    // 获取数据
    pGamePropertySubItem = m_GamePropertyListManager.EmunGamePropertySubItem(Position);
    if (pGamePropertySubItem == nullptr)
      break;

    // 拷贝数据
    CopyMemory(cbDataBuffer + wSendSize, &pGamePropertySubItem->m_PropertySubItem, sizeof(tagPropertySubItem));
    wSendSize += sizeof(tagPropertySubItem);
  }

  // 发送剩余
  if (wSendSize > 0)
    network_engine_->SendData(dwSocketID, MDM_GP_SERVER_LIST, SUB_GP_LIST_PROPERTY_SUB, cbDataBuffer, wSendSize);
}

// 会员配置
VOID CDispatchEngineSink::SendMemberConfig(DWORD dwContextID) {
  // 会员配置
  CMD_GP_MemberParameterResult MemberParameterResult;
  MemberParameterResult.wMemberCount = m_wMemberCount;
  CopyMemory(MemberParameterResult.MemberParameter, m_MemberParameter, sizeof(tagMemberParameterNew) * m_wMemberCount);

  WORD wConfigMemberHead = sizeof(MemberParameterResult) - sizeof(MemberParameterResult.MemberParameter);
  WORD wConfigMemberInfo = MemberParameterResult.wMemberCount * sizeof(MemberParameterResult.MemberParameter[0]);
  network_engine_->SendData(dwContextID, MDM_GP_LOGON, SUB_GP_MEMBER_PARAMETER_RESULT, &MemberParameterResult, wConfigMemberHead + wConfigMemberInfo);
}

VOID CDispatchEngineSink::SendRealAuthConfig(DWORD dwContextID) {
  // 实名认证
  CMD_GP_RealAuthParameter CmdParameter;
  CopyMemory(&CmdParameter, &m_AuthRealParameter, sizeof(CmdParameter));
  network_engine_->SendData(dwContextID, MDM_GP_LOGON, SUB_GP_REAL_AUTH_CONFIG, &CmdParameter, sizeof(CmdParameter));
}

// 发送类型
VOID CDispatchEngineSink::SendMobileKindInfo(DWORD dwSocketID, WORD wModuleID) {
}

// 发送房间
VOID CDispatchEngineSink::SendMobileServerInfo(DWORD dwSocketID, WORD wModuleID) {
  // 网络数据
  WORD wSendSize = 0;
  BYTE cbDataBuffer[SOCKET_TCP_PACKET];

  // 枚举数据
  CServerItemMap::iterator* Position = nullptr;
  CGameServerItem* pGameServerItem = nullptr;
  CGameKindItem* pGameKindItem = nullptr;
  // 枚举数据
  for (DWORD i = 0; i < m_ServerListManager.GetGameServerCount(); i++) {
    // 发送数据
    if ((wSendSize + sizeof(tagGameServer)) > sizeof(cbDataBuffer)) {
      network_engine_->SendData(dwSocketID, MDM_MB_SERVER_LIST, SUB_MB_LIST_SERVER, cbDataBuffer, wSendSize);
      wSendSize = 0;
    }

    // 获取数据
    pGameServerItem = m_ServerListManager.EmunGameServerItem(Position);
    if (pGameServerItem == nullptr)
      break;

    // 支持类型
    bool bServerSupportMobile = CServerRule::IsSupportMobile(pGameServerItem->m_GameServer.dwServerRule);

    // 拷贝数据
    if ((bServerSupportMobile == true) && ((wModuleID == INVALID_WORD) || (pGameServerItem->m_GameServer.wKindID == wModuleID))) {
      CopyMemory(cbDataBuffer + wSendSize, &pGameServerItem->m_GameServer, sizeof(tagGameServer));
      wSendSize += sizeof(tagGameServer);
    }
  }

  // 发送剩余
  if (wSendSize > 0) {
    network_engine_->SendData(dwSocketID, MDM_MB_SERVER_LIST, SUB_MB_LIST_SERVER, cbDataBuffer, wSendSize);
  }

  // 设置变量
  wSendSize = 0;
  ZeroMemory(cbDataBuffer, sizeof(cbDataBuffer));

  // 枚举数据
  for (DWORD i = 0; i < m_ServerListManager.GetGameServerCount(); i++) {
    // 发送数据
    if ((wSendSize + sizeof(tagGameMatch)) > sizeof(cbDataBuffer)) {
      network_engine_->SendData(dwSocketID, MDM_MB_SERVER_LIST, SUB_MB_LIST_MATCH, cbDataBuffer, wSendSize);
      wSendSize = 0;
    }

    // 获取数据
    pGameServerItem = m_ServerListManager.EmunGameServerItem(Position);
    if (pGameServerItem == nullptr)
      break;
    if (pGameServerItem->IsMatchServer() == false)
      continue;

    // 拷贝数据
    if ((wModuleID == INVALID_WORD) || (pGameServerItem->m_GameServer.wKindID == wModuleID)) {
      CopyMemory(cbDataBuffer + wSendSize, &pGameServerItem->m_GameMatch, sizeof(tagGameMatch));
      wSendSize += sizeof(tagGameMatch);
    }
  }

  // 发送剩余
  if (wSendSize > 0) {
    network_engine_->SendData(dwSocketID, MDM_MB_SERVER_LIST, SUB_MB_LIST_MATCH, cbDataBuffer, wSendSize);
  }
}

// 银行失败
bool CDispatchEngineSink::SendInsureFailure(DWORD dwSocketID, LONG lResultCode, LPCTSTR pszDescribe) {
  // 变量定义
  CMD_GP_UserInsureFailure UserInsureFailure;
  ZeroMemory(&UserInsureFailure, sizeof(UserInsureFailure));

  // 构造数据
  UserInsureFailure.lResultCode = lResultCode;
  LSTRCPYN(UserInsureFailure.szDescribeString, pszDescribe, std::size(UserInsureFailure.szDescribeString));

  // 发送数据
  WORD wDescribe = CountStringBuffer(UserInsureFailure.szDescribeString);
  WORD wHeadSize = sizeof(UserInsureFailure) - sizeof(UserInsureFailure.szDescribeString);
  network_engine_->SendData(dwSocketID, MDM_GP_USER_SERVICE, SUB_GP_USER_INSURE_FAILURE, &UserInsureFailure, wHeadSize + wDescribe);

  // 关闭连接
  network_engine_->ShutDownSocket(dwSocketID);

  return true;
}

// 操作成功
VOID CDispatchEngineSink::SendOperateSuccess(DWORD dwContextID, LONG lResultCode, LPCTSTR pszDescribe) {
  // 效验参数
  ASSERT(pszDescribe != nullptr);
  if (pszDescribe == nullptr)
    return;

  // 变量定义
  CMD_GP_OperateSuccess OperateSuccess;
  ZeroMemory(&OperateSuccess, sizeof(OperateSuccess));

  // 构造数据
  OperateSuccess.lResultCode = lResultCode;
  LSTRCPYN(OperateSuccess.szDescribeString, pszDescribe, std::size(OperateSuccess.szDescribeString));

  // 发送数据
  WORD wDescribe = CountStringBuffer(OperateSuccess.szDescribeString);
  WORD wHeadSize = sizeof(OperateSuccess) - sizeof(OperateSuccess.szDescribeString);
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_OPERATE_SUCCESS, &OperateSuccess, wHeadSize + wDescribe);

  // 关闭连接
  network_engine_->ShutDownSocket(dwContextID);
}

// 操作失败
VOID CDispatchEngineSink::SendOperateFailure(DWORD dwContextID, LONG lResultCode, LPCTSTR pszDescribe) {
  // 效验参数
  ASSERT(pszDescribe != nullptr);
  if (pszDescribe == nullptr)
    return;

  // 变量定义
  CMD_GP_OperateFailure OperateFailure;
  ZeroMemory(&OperateFailure, sizeof(OperateFailure));

  // 构造数据
  OperateFailure.lResultCode = lResultCode;
  LSTRCPYN(OperateFailure.szDescribeString, pszDescribe, std::size(OperateFailure.szDescribeString));

  // 发送数据
  WORD wDescribe = CountStringBuffer(OperateFailure.szDescribeString);
  WORD wHeadSize = sizeof(OperateFailure) - sizeof(OperateFailure.szDescribeString);
  network_engine_->SendData(dwContextID, MDM_GP_USER_SERVICE, SUB_GP_OPERATE_FAILURE, &OperateFailure, wHeadSize + wDescribe);

  // 关闭连接
  network_engine_->ShutDownSocket(dwContextID);
}

// 生成验证码
VOID CDispatchEngineSink::RandVerifyCode(LPTSTR pszVerifyCode, UINT nMaxCount) {
  ASSERT(pszVerifyCode != nullptr && nMaxCount >= LEN_VERIFY_CODE);
  if (pszVerifyCode == nullptr || nMaxCount < LEN_VERIFY_CODE)
    return;

  for (UINT i = 0; i < nMaxCount - 1; i++) {
    INT nflag = rand() % 3;
    switch (nflag) {
      case 0:
        pszVerifyCode[i] = TEXT('A') + rand() % 26;
        break;
      case 1:
        pszVerifyCode[i] = TEXT('a') + rand() % 26;
        break;
      case 2:
        pszVerifyCode[i] = TEXT('0') + rand() % 10;
        break;
      default:
        pszVerifyCode[i] = TEXT('x');
        break;
    }
  }
  pszVerifyCode[nMaxCount - 1] = TEXT('\0');
}

// 效验验证
bool CDispatchEngineSink::CheckVerifyCode(LPCTSTR pszValidateCode, LPCTSTR pszValidateCodeBak) {
  // 长度验证
  if (pszValidateCode == nullptr || pszValidateCodeBak == nullptr)
    return false;

  // 长度对比
  INT nTargetLen = StrLenT(pszValidateCode);
  INT nSourceLen = StrLenT(pszValidateCodeBak);

  // 密码对比
  if (nTargetLen != nSourceLen)
    return false;
  return std::equal(pszValidateCode, pszValidateCode + nTargetLen, pszValidateCodeBak, [](const char c1, const char c2) {
    return (c1 == c2 || std::toupper(c1) == std::toupper(c2));
  });
}

// 发送验证码
VOID CDispatchEngineSink::SendVerifyCode(DWORD dwSocketID, LPCTSTR pszVerifyCode, bool bCloseSocket) {
  // 效验参数
  ASSERT(pszVerifyCode != nullptr);
  if (pszVerifyCode == nullptr)
    return;

  // 变量定义
  CMD_GP_QueryVerifyCode QueryVerifyCode;
  ZeroMemory(&QueryVerifyCode, sizeof(QueryVerifyCode));

  // 构造数据
  QueryVerifyCode.bCloseSocket = bCloseSocket;
  LSTRCPYN(QueryVerifyCode.szVerifyCode, pszVerifyCode, std::size(QueryVerifyCode.szVerifyCode));

  // 发送数据
  network_engine_->SendData(dwSocketID, MDM_GP_LOGON, SUB_GP_QUERY_VERIFY_CODE, &QueryVerifyCode, sizeof(QueryVerifyCode));
}

// 私人房间请求结束
bool CDispatchEngineSink::OnDBQueryPersonalRoomEnd(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 发送数据
  network_engine_->SendData(dwContextID, MDM_MB_PERSONAL_SERVICE, SUB_MB_QUERY_PERSONAL_END, nullptr, 0);

  return true;
}
//////////////////////////////////////////////////////////////////////////////////
