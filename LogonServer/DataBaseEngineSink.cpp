#include "DataBaseEngineSink.h"
#include "ServiceUnits.h"
#include <KernelEngine/AsyncEventHub.h>

//////////////////////////////////////////////////////////////////////////////////

// 接口查询
VOID* CDataBaseEngineSink::QueryInterface(REFGUID Guid, DWORD dwQueryVer) {
  QUERYINTERFACE(IDataBaseEngineSink, Guid, dwQueryVer);
  QUERYINTERFACE_IUNKNOWNEX(IDataBaseEngineSink, Guid, dwQueryVer);
  return nullptr;
}

// 启动事件
bool CDataBaseEngineSink::OnDataBaseEngineStart(IUnknownEx* pIUnknownEx) {
  // 创建对象
  if ((m_AccountsDBModule.GetInterface() == nullptr) && (!m_AccountsDBModule.CreateInstance())) {
    ASSERT(FALSE);
    return false;
  }

  // 创建对象
  if ((m_TreasureDBModule.GetInterface() == nullptr) && (!m_TreasureDBModule.CreateInstance())) {
    ASSERT(FALSE);
    return false;
  }

  // 创建对象
  if ((m_PlatformDBModule.GetInterface() == nullptr) && (!m_PlatformDBModule.CreateInstance())) {
    ASSERT(FALSE);
    return false;
  }

  try {
    // 连接信息
    tagDataBaseParameter* pAccountsDBParameter = &init_parameter_->accounts_db_parameter_;
    tagDataBaseParameter* pTreasureDBParameter = &init_parameter_->treasure_db_parameter_;
    tagDataBaseParameter* pPlatformDBParameter = &init_parameter_->platform_db_parameter_;

    // 设置连接
    m_AccountsDBModule->SetConnectionInfo(pAccountsDBParameter->szDataBaseAddr, pAccountsDBParameter->wDataBasePort,
                                          pAccountsDBParameter->szDataBaseName, pAccountsDBParameter->szDataBaseUser,
                                          pAccountsDBParameter->szDataBasePass);
    m_TreasureDBModule->SetConnectionInfo(pTreasureDBParameter->szDataBaseAddr, pTreasureDBParameter->wDataBasePort,
                                          pTreasureDBParameter->szDataBaseName, pTreasureDBParameter->szDataBaseUser,
                                          pTreasureDBParameter->szDataBasePass);
    m_PlatformDBModule->SetConnectionInfo(pPlatformDBParameter->szDataBaseAddr, pPlatformDBParameter->wDataBasePort,
                                          pPlatformDBParameter->szDataBaseName, pPlatformDBParameter->szDataBaseUser,
                                          pPlatformDBParameter->szDataBasePass);

    // 发起连接
    m_AccountsDBModule->OpenConnection();
    m_AccountsDBAide.SetDataBase(m_AccountsDBModule.GetInterface());

    // 发起连接
    m_TreasureDBModule->OpenConnection();
    m_TreasureDBAide.SetDataBase(m_TreasureDBModule.GetInterface());

    // 发起连接
    m_PlatformDBModule->OpenConnection();
    m_PlatformDBAide.SetDataBase(m_PlatformDBModule.GetInterface());

    return true;
  } catch (IDataBaseException* pIException) {
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

// 停止事件
bool CDataBaseEngineSink::OnDataBaseEngineConclude(IUnknownEx* pIUnknownEx) {
  // 设置对象
  m_AccountsDBAide.SetDataBase(nullptr);
  m_TreasureDBAide.SetDataBase(nullptr);
  m_PlatformDBAide.SetDataBase(nullptr);

  // 关闭连接
  if (m_AccountsDBModule.GetInterface() != nullptr) {
    m_AccountsDBModule->CloseConnection();
    m_AccountsDBModule.CloseInstance();
  }

  // 关闭连接
  if (m_TreasureDBModule.GetInterface() != nullptr) {
    m_TreasureDBModule->CloseConnection();
    m_TreasureDBModule.CloseInstance();
  }

  // 关闭连接
  if (m_PlatformDBModule.GetInterface() != nullptr) {
    m_PlatformDBModule->CloseConnection();
    m_PlatformDBModule.CloseInstance();
  }

  // 组件变量
  init_parameter_ = nullptr;
  // m_pIDataBaseEngineEvent = nullptr;

  return true;
}

// 时间事件
bool CDataBaseEngineSink::OnDataBaseEngineTimer(DWORD dwTimerID, WPARAM dwBindParameter) { return false; }

// 控制事件
bool CDataBaseEngineSink::OnDataBaseEngineControl(WORD wControlID, VOID* pData, WORD wDataSize) { return false; }

bool CDataBaseEngineSink::EmitDataBaseEngineResult(WORD wRequestID, DWORD dwContextID, VOID* pData, WORD wDataSize) {
  auto post_data = ConvertToBytes(pData, wDataSize);
  GlobalEventBus::Get()->Publish<DataBaseResultEventTag>(wRequestID, dwContextID, post_data);
  return true;
}

// 请求事件
bool CDataBaseEngineSink::OnDataBaseEngineRequest(WORD wRequestID, DWORD dwContextID, VOID* pData, WORD wDataSize) {
  switch (wRequestID) {
  case DBR_GP_LOGON_GAMEID: // 标识登录
  {
    return OnRequestLogonGameID(dwContextID, pData, wDataSize);
  }
  case DBR_GP_LOGON_ACCOUNTS: // 帐号登录
  {
    return OnRequestLogonAccounts(dwContextID, pData, wDataSize);
  }
  case DBR_GP_REGISTER_ACCOUNTS: // 注册帐号
  {
    return OnRequestRegisterAccounts(dwContextID, pData, wDataSize);
  }
  case DBR_GP_LOAD_PLATFORM_PARAMETER: // 平台参数
  {
    return OnRequestPlatformParameter(dwContextID, pData, wDataSize);
  }
  case DBR_GP_VERIFY_INDIVIDUAL: // 验证资料
  {
    return OnRequestVerifyIndividual(dwContextID, pData, wDataSize);
  }
  case DBR_GP_LOGON_VISITOR: // 游客登陆
  {
    return OnRequestLogonVisitor(dwContextID, pData, wDataSize);
  }
  case DBR_GP_MODIFY_MACHINE: // 修改机器
  {
    return OnRequestModifyMachine(dwContextID, pData, wDataSize);
  }
  case DBR_GP_MODIFY_LOGON_PASS: // 修改密码
  {
    return OnRequestModifyLogonPass(dwContextID, pData, wDataSize);
  }
  case DBR_GP_MODIFY_INSURE_PASS: // 修改密码
  {
    return OnRequestModifyInsurePass(dwContextID, pData, wDataSize);
  }
  case DBR_GP_MODIFY_UNDER_WRITE: // 修改签名
  {
    return OnRequestModifyUnderWrite(dwContextID, pData, wDataSize);
  }
  case DBR_GP_MODIFY_REAL_AUTH: // 实名认证
  {
    return OnRequestModifyRealAuth(dwContextID, pData, wDataSize);
  }
  case DBR_GP_MODIFY_SYSTEM_FACE: // 修改头像
  {
    return OnRequestModifySystemFace(dwContextID, pData, wDataSize);
  }
  case DBR_GP_MODIFY_CUSTOM_FACE: // 修改头像
  {
    return OnRequestModifyCustomFace(dwContextID, pData, wDataSize);
  }
  case DBR_GP_MODIFY_INDIVIDUAL: // 修改资料
  {
    return OnRequestModifyIndividual(dwContextID, pData, wDataSize);
  }
  case DBR_GP_USER_ENABLE_INSURE: // 开通银行
  {
    return OnRequestUserEnableInsure(dwContextID, pData, wDataSize);
  }
  case DBR_GP_USER_SAVE_SCORE: // 存入游戏币
  {
    return OnRequestUserSaveScore(dwContextID, pData, wDataSize);
  }
  case DBR_GP_USER_TAKE_SCORE: // 提取游戏币
  {
    return OnRequestUserTakeScore(dwContextID, pData, wDataSize);
  }
  case DBR_GP_USER_TRANSFER_SCORE: // 转帐游戏币
  {
    return OnRequestUserTransferScore(dwContextID, pData, wDataSize);
  }
  case DBR_GP_QUERY_INDIVIDUAL: // 查询资料
  {
    return OnRequestQueryIndividual(dwContextID, pData, wDataSize);
  }
  case DBR_GP_BIND_SPREADER: // 绑定推广
  {
    return OnRequestBindSpreader(dwContextID, pData, wDataSize);
  }
  case DBR_GP_QUERY_INSURE_INFO: // 查询银行
  {
    return OnRequestQueryInsureInfo(dwContextID, pData, wDataSize);
  }
  case DBR_GP_QUERY_USER_INFO: // 查询用户
  {
    return OnRequestQueryTransferUserInfo(dwContextID, pData, wDataSize);
  }
  case DBR_GP_QUERY_TRANSFER_REBATE: // 查询返利
  {
    return OnRequestQueryTransferRebate(dwContextID, pData, wDataSize);
  }
  case DBR_GP_LOAD_GAME_LIST: // 加载列表
  {
    return OnRequestLoadGameList(dwContextID, pData, wDataSize);
  }
  case DBR_GP_ONLINE_COUNT_INFO: // 在线信息
  {
    return OnRequestOnLineCountInfo(dwContextID, pData, wDataSize);
  }
  case DBR_GP_LOAD_CHECKIN_REWARD: // 签到奖励
  {
    return OnRequestCheckInReward(dwContextID, pData, wDataSize);
  }
  case DBR_GP_LOAD_TASK_LIST: // 加载任务
  {
    return OnRequestLoadTaskList(dwContextID, pData, wDataSize);
  }
  case DBR_GP_LOAD_BASEENSURE: // 加载低保
  {
    return OnRequestLoadBaseEnsure(dwContextID, pData, wDataSize);
  }
  case DBR_GP_QUERY_SPREAD_INFO: // 推广信息
  {
    return OnRequestQuerySpreadInfo(dwContextID, pData, wDataSize);
  }
  case DBR_GP_LOAD_REAL_AUTH: // 加载认证
  {
    return OnRequestLoadRealAuth(dwContextID, pData, wDataSize);
  }
  case DBR_GP_GET_PARAMETER: // 获取参数
  {
    return OnRequestGetParameter(dwContextID, pData, wDataSize);
  }
  case DBR_GP_ADD_PARAMETER: // 添加参数
  {
    return OnRequestAddParameter(dwContextID, pData, wDataSize);
  }
  case DBR_GP_MODIFY_PARAMETER: // 修改参数
  {
    return OnRequestModifyParameter(dwContextID, pData, wDataSize);
  }
  case DBR_GP_DELETE_PARAMETER: // 删除参数
  {
    return OnRequestDeleteParameter(dwContextID, pData, wDataSize);
  }
  case DBR_GP_CHECKIN_QUERY_INFO: // 查询签到
  {
    return OnRequestCheckInQueryInfo(dwContextID, pData, wDataSize);
  }
  case DBR_GP_CHECKIN_DONE: // 执行签到
  {
    return OnRequestCheckInDone(dwContextID, pData, wDataSize);
  }
  case DBR_GP_TASK_QUERY_INFO: // 查询任务
  {
    return OnRequestTaskQueryInfo(dwContextID, pData, wDataSize);
  }
  case DBR_GP_TASK_GIVEUP: // 放弃任务
  {
    return OnRequestTaskGiveUp(dwContextID, pData, wDataSize);
  }
  case DBR_GP_TASK_TAKE: // 领取任务
  {
    return OnRequestTaskTake(dwContextID, pData, wDataSize);
  }
  case DBR_GP_TASK_REWARD: // 领取奖励
  {
    return OnRequestTaskReward(dwContextID, pData, wDataSize);
  }
  case DBR_GP_BASEENSURE_TAKE: // 领取低保
  {
    return OnRequestTakeBaseEnsure(dwContextID, pData, wDataSize);
  }
  case DBR_GP_GROWLEVEL_QUERY_IFNO: // 查询等级
  {
    return OnRequestQueryGrowLevelParameter(dwContextID, pData, wDataSize);
  }
  case DBR_GP_LOAD_MEMBER_PARAMETER: // 会员参数
  {
    return OnRequestMemberLoadParameter(dwContextID, pData, wDataSize);
  }
  case DBR_GP_MEMBER_QUERY_INFO: // 会员查询
  {
    return OnRequestMemberQueryInfo(dwContextID, pData, wDataSize);
  }
  case DBR_GP_MEMBER_DAY_PRESENT: // 会员赠送
  {
    return OnRequestMemberDayPresent(dwContextID, pData, wDataSize);
  }
  case DBR_GP_MEMBER_DAY_GIFT: // 会员礼物
  {
    return OnRequestMemberDayGift(dwContextID, pData, wDataSize);
  }
  case DBR_GP_LOAD_GROWLEVEL_CONFIG: // 等级配置
  {
    return OnRequestLoadGrowLevelConfig(dwContextID, pData, wDataSize);
  }
  case DBR_GP_LOAD_GAME_PROPERTY_LIST: // 加载道具
  {
    return OnRequestLoadGamePropertyList(dwContextID, pData, wDataSize);
  }
  case DBR_GP_PROPERTY_BUY: // 购买道具
  {
    return OnRequestBuyGameProperty(dwContextID, pData, wDataSize);
  }
  case DBR_GP_PROPERTY_USE: // 使用道具
  {
    return OnRequestUseProperty(dwContextID, pData, wDataSize);
  }
  case DBR_GP_USER_PROPERTY_BUFF: // 加载玩家Buff
  {
    return OnRequestPropertyBuff(dwContextID, pData, wDataSize);
  }
  case DBR_GP_QUERY_SINGLE: // 查询背包
  {
    return OnRequestPropertyQuerySingle(dwContextID, pData, wDataSize);
  }
  case DBR_GP_QUERY_BACKPACK: // 查询背包
  {
    return OnRequestUserBackpackProperty(dwContextID, pData, wDataSize);
  }
  case DBR_GP_PROPERTY_PRESENT: // 赠送道具
  {
    return OnRequestPropertyPresent(dwContextID, pData, wDataSize);
  }
  case DBR_GP_QUERY_SEND_PRESENT: // 查询赠送
  {
    return OnRequestQuerySendPresent(dwContextID, pData, wDataSize);
  }
  case DBR_GP_GET_SEND_PRESENT: // 获取赠送
  {
    return OnRequestGetSendPresent(dwContextID, pData, wDataSize);
  }
  case DBR_GP_PURCHASE_MEMBER: // 购买会员
  {
    return OnRequestPurchaseMember(dwContextID, pData, wDataSize);
  }
  case DBR_GP_EXCHANGE_SCORE_INGOT: // 兑换游戏币
  {
    return OnRequestExchangeScoreByIngot(dwContextID, pData, wDataSize);
  }
  case DBR_GP_EXCHANGE_SCORE_BEANS: // 兑换游戏币
  {
    return OnRequestExchangeScoreByBeans(dwContextID, pData, wDataSize);
  }
  case DBR_MB_LOGON_GAMEID: // 标识登录
  {
    return OnMobileLogonGameID(dwContextID, pData, wDataSize);
  }
  case DBR_MB_LOGON_ACCOUNTS: // 帐号登录
  {
    return OnMobileLogonAccounts(dwContextID, pData, wDataSize);
  }
  case DBR_MB_REGISTER_ACCOUNTS: // 注册帐号
  {
    return OnMobileRegisterAccounts(dwContextID, pData, wDataSize);
  }
  case DBR_MB_LOGON_OTHERPLATFORM: // 其他登录
  {
    return OnMobileLogonOtherPlatform(dwContextID, pData, wDataSize);
  }
  case DBR_MB_LOGON_VISITOR: // 游客登陆
  {
    return OnMobileLogonVisitor(dwContextID, pData, wDataSize);
  }
  case DBR_GP_LOTTERY_CONFIG_REQ: // 请求配置
  {
    return OnRequestLotteryConfigReq(dwContextID, pData, wDataSize);
  }
  case DBR_GP_LOTTERY_START: // 抽奖开始
  {
    return OnRequestLotteryStart(dwContextID, pData, wDataSize);
  }
  case DBR_GP_QUERY_USER_GAME_DATA: // 游戏数据
  {
    return OnRequestQueryUserGameData(dwContextID, pData, wDataSize);
  }
  case DBR_GP_ACCOUNT_BIND: {
    return OnRequestAccountBind(dwContextID, pData, wDataSize);
  }
  case DBR_GP_ACCOUNT_BIND_EXISTS: {
    return OnRequestAccountBindExists(dwContextID, pData, wDataSize);
  }
  case DBR_MB_GET_PERSONAL_PARAMETER: // 约战房间参数
  {
    return OnRequestGetPersonalParameter(dwContextID, pData, wDataSize);
  }
  case DBR_MB_QUERY_PERSONAL_ROOM_INFO: // 约战房间信息
  {
    return OnRequestqueryPersonalRoomInfo(dwContextID, pData, wDataSize);
  }
  case DBR_GR_QUERY_USER_ROOM_SCORE: // 约战房间分数
  {
    return OnRequestQueryUserRoomScore(dwContextID, pData, wDataSize);
  }
  case DBR_GR_CLOSE_ROOM_WRITE_DISSUME_TIME: // 写入解散约战房间时间
  {
    return OnRequestCloseRoomWriteDissumeTime(dwContextID, pData, wDataSize);
  }
  case DBR_MB_QUERY_PERSONAL_ROOM_USER_INFO: {
    return OnRequestPersonalRoomUserInfo(dwContextID, pData, wDataSize);
  }
  case DBR_MB_ROOM_CARD_EXCHANGE_TO_SCORE: {
    return OnRequestRoomCardExchangeToScore(dwContextID, pData, wDataSize);
  }
  case DBR_MB_QUERY_VIDEO_INFO: {
    return OnRequestVideoInfo(dwContextID, pData, wDataSize);
  }
  case DBR_MB_QUERY_VIDEO_DETAILS: {
    return OnRequestVideoDetails(dwContextID, pData, wDataSize);
  }
  case DBR_MB_QUERY_VIDEO_DETAILS_BY_ROOMID: {
    return OnRequestVideoDetailsByRoomID(dwContextID, pData, wDataSize);
  }
  case DBR_MB_QUERY_PLAYBACK_CODE_YZ: {
    return OnRequestPlayBackCodeYZ(dwContextID, pData, wDataSize);
  }
  case DBR_MB_QUERY_PLAYBACK_CODE: {
    return OnRequestPlayBackCode(dwContextID, pData, wDataSize);
  }
  case DBR_MB_GET_PERSONAL_RULE: {
    return OnRequestQueryPersonalRule(dwContextID, pData, wDataSize);
  }
  case DBR_GP_CREATE_CLUB: {
    return OnRequestCreateClub(dwContextID, pData, wDataSize);
  }
  case DBR_GP_QUERY_CLUB_LIST: {
    return OnRequestGetClubList(dwContextID, pData, wDataSize);
  }
  case DBR_GP_QUERY_CLUB_USER_LIST: {
    return OnRequestGetClubUserList(dwContextID, pData, wDataSize);
  }
  case DBR_GP_QUERY_CLUB_NAME: {
    return OnRequestGetClubName(dwContextID, pData, wDataSize);
  }
  case DBR_GP_QUERY_JOIN_CLUB: {
    return OnRequestQueryJoinClub(dwContextID, pData, wDataSize);
  }
  case DBR_GP_QUERY_QUIT_CLUB: {
    return OnRequestQueryQuitClub(dwContextID, pData, wDataSize);
  }
  case DBR_GP_QUERY_CLUB_JOINUSER_LIST: {
    return OnRequestQueryClubJoinUserList(dwContextID, pData, wDataSize);
  }
  case DBR_GP_DEAL_USER_JOIN_CLUB: {
    return OnRequestDealUserJoinClub(dwContextID, pData, wDataSize);
  }
  case DBR_GP_MOVE_JEWEL_TO_CLUB: {
    return OnRequestMoveJewelToClub(dwContextID, pData, wDataSize);
  }
  case DBR_GP_DELETE_CLUB_USER: {
    return OnRequestDeleteClubUser(dwContextID, pData, wDataSize);
  }
  case DBR_GP_CREATE_CLUB_ROOM_RULE: {
    return OnRequestCreateClubRoomRule(dwContextID, pData, wDataSize);
  }
  case DBR_GP_GET_CLUB_ALL_ROOM_RULE: {
    return OnRequestGetClubAllRoomRule(dwContextID, pData, wDataSize);
  }
  case DBR_GP_DELETE_CLUB_ROOM_RULE: {
    return OnRequestDeleteClubRoomRule(dwContextID, pData, wDataSize);
  }
  case DBR_GP_GET_CLUB_ROOM_LIST: {
    return OnRequestGetClubRoomList(dwContextID, pData, wDataSize);
  }
  case DBR_GP_MODIFY_CLUB_SUMMARY: {
    return OnRequestModifyClubSummary(dwContextID, pData, wDataSize);
  }
  case DBR_GP_JOIN_USER_TO_CLUB: {
    return OnRequestJoinUserToClub(dwContextID, pData, wDataSize);
  }
  }

  return false;
}

// I D 登录
bool CDataBaseEngineSink::OnRequestLogonGameID(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_LogonGameID));
    if (wDataSize != sizeof(DBR_GP_LogonGameID))
      return false;

    // 执行查询
    DBR_GP_LogonGameID* pLogonGameID = (DBR_GP_LogonGameID*)pData;

    // 变量定义
    tagBindParameter* pBindParameter = (tagBindParameter*)pLogonGameID->pBindParameter;
    if (pBindParameter->dwSocketID != dwContextID)
      return true;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pLogonGameID->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwGameID"), pLogonGameID->dwGameID);
    m_AccountsDBAide.AddParameter(TEXT("@strPassword"), pLogonGameID->szPassword);
    m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_AccountsDBAide.AddParameter(TEXT("@strMachineID"), pLogonGameID->szMachineID);
    // m_AccountsDBAide.AddParameter(TEXT("@nNeeValidateMBCard"),pLogonGameID->cbNeeValidateMBCard);
    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_EfficacyGameID"), true);

    // 结果处理
    StringT DBVarValue;
    m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
    OnLogonDisposeResult(dwContextID, lResultCode, DBVarValue.c_str(), 0, false);

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);

    // 错误处理
    OnLogonDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试或选择另一服务器登录！"), 0, false);

    return false;
  }

  return true;
}

// 帐号登录
bool CDataBaseEngineSink::OnRequestLogonAccounts(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_LogonAccounts));
    if (wDataSize != sizeof(DBR_GP_LogonAccounts))
      return false;

    // 请求处理
    DBR_GP_LogonAccounts* pLogonAccounts = (DBR_GP_LogonAccounts*)pData;

    // 执行判断
    tagBindParameter* pBindParameter = (tagBindParameter*)pLogonAccounts->pBindParameter;
    if (pBindParameter->dwSocketID != dwContextID)
      return true;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pLogonAccounts->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 变量定义
    LONG lResultCode = DB_SUCCESS;

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");

    // 检查权限
    if (pLogonAccounts->dwCheckUserRight != 0) {
      // 构造参数
      m_AccountsDBAide.ResetParameter();
      m_AccountsDBAide.AddParameter(TEXT("@strAccounts"), pLogonAccounts->szAccounts);
      m_AccountsDBAide.AddParameter(TEXT("@dwcheckRight"), pLogonAccounts->dwCheckUserRight);
      m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

      // 执行过程
      lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_CheckUserRight"), false);
    }

    // 验证帐号
    if (lResultCode == DB_SUCCESS) {
      // 构造参数
      m_AccountsDBAide.ResetParameter();
      m_AccountsDBAide.AddParameter(TEXT("@strAccounts"), pLogonAccounts->szAccounts);
      m_AccountsDBAide.AddParameter(TEXT("@strPassword"), pLogonAccounts->szPassword);
      m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
      m_AccountsDBAide.AddParameter(TEXT("@strMachineID"), pLogonAccounts->szMachineID);
      // m_AccountsDBAide.AddParameter(TEXT("@nNeeValidateMBCard"),pLogonAccounts->cbNeeValidateMBCard);
      // m_AccountsDBAide.AddParameter(TEXT("@strPassPortID"),pLogonAccounts->szPassPortID);
      m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

      // 执行查询
      lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_EfficacyAccounts"), true);
    }

    // 结果处理
    StringT DBVarValue;
    m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
    OnLogonDisposeResult(dwContextID, lResultCode, DBVarValue.c_str(), pLogonAccounts->dwCheckUserRight, false);

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);

    // 错误处理
    OnLogonDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试或选择另一服务器登录！"), 0, false);

    return false;
  }

  return true;
}

// 注册处理
bool CDataBaseEngineSink::OnRequestRegisterAccounts(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_RegisterAccounts));
    if (wDataSize != sizeof(DBR_GP_RegisterAccounts))
      return false;

    // 请求处理
    DBR_GP_RegisterAccounts* pRegisterAccounts = (DBR_GP_RegisterAccounts*)pData;

    // 执行判断
    tagBindParameter* pBindParameter = (tagBindParameter*)pRegisterAccounts->pBindParameter;
    if (pBindParameter->dwSocketID != dwContextID)
      return true;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pRegisterAccounts->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@strAccounts"), pRegisterAccounts->szAccounts);
    m_AccountsDBAide.AddParameter(TEXT("@strNickName"), pRegisterAccounts->szNickName);
    m_AccountsDBAide.AddParameter(TEXT("@dwSpreaderGameID"), pRegisterAccounts->dwSpreaderGameID);
    m_AccountsDBAide.AddParameter(TEXT("@strLogonPass"), pRegisterAccounts->szLogonPass);
    m_AccountsDBAide.AddParameter(TEXT("@wFaceID"), pRegisterAccounts->wFaceID);
    m_AccountsDBAide.AddParameter(TEXT("@cbGender"), pRegisterAccounts->cbGender);
    m_AccountsDBAide.AddParameter(TEXT("@strPassPortID"), pRegisterAccounts->szPassPortID);
    m_AccountsDBAide.AddParameter(TEXT("@strCompellation"), pRegisterAccounts->szCompellation);
    m_AccountsDBAide.AddParameter(TEXT("@dwAgentID"), pRegisterAccounts->dwAgentID);
    m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_AccountsDBAide.AddParameter(TEXT("@strMachineID"), pRegisterAccounts->szMachineID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_RegisterAccounts"), true);

    // 结果处理
    StringT DBVarValue;
    m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
    OnLogonDisposeResult(dwContextID, lResultCode, DBVarValue.c_str(), 0, false);

    return true;
  } catch (IDataBaseException* pIException) {
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);

    // 错误处理
    OnLogonDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试或选择另一服务器登录！"), 0, false);

    return false;
  }

  return true;
}

// 验证资料
bool CDataBaseEngineSink::OnRequestVerifyIndividual(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_VerifyIndividual));
    if (wDataSize != sizeof(DBR_GP_VerifyIndividual))
      return false;

    // 请求处理
    DBR_GP_VerifyIndividual* pVerifyIndividual = (DBR_GP_VerifyIndividual*)pData;

    // 执行判断
    tagBindParameter* pBindParameter = (tagBindParameter*)pVerifyIndividual->pBindParameter;
    if (pBindParameter->dwSocketID != dwContextID)
      return true;

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@wVerifyMask"), pVerifyIndividual->wVerifyMask);
    m_AccountsDBAide.AddParameter(TEXT("@strVerifyContent"), pVerifyIndividual->szVerifyContent);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_VerifyIndividual"), false);

    // 结果处理
    DBO_GP_VerifyIndividualResult VerifyIndividualResult;
    VerifyIndividualResult.szErrorMsg[0] = 0;
    VerifyIndividualResult.wVerifyMask = pVerifyIndividual->wVerifyMask;
    VerifyIndividualResult.bVerifyPassage = lResultCode == DB_SUCCESS;

    // 结果处理
    StringT DBVarValue;
    m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
    LSTRCPYN(VerifyIndividualResult.szErrorMsg, DBVarValue.c_str(), std::size(VerifyIndividualResult.szErrorMsg));

    // 发送结果
    WORD wDataSize = CountStringBuffer(VerifyIndividualResult.szErrorMsg);
    WORD wHeadSize = sizeof(VerifyIndividualResult) - sizeof(VerifyIndividualResult.szErrorMsg);
    EmitDataBaseEngineResult(DBO_GP_VERIFY_RESULT, dwContextID, &VerifyIndividualResult, wHeadSize + wDataSize);

    return true;
  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);

    // 请求处理
    DBR_GP_VerifyIndividual* pVerifyIndividual = (DBR_GP_VerifyIndividual*)pData;

    // 结果处理
    DBO_GP_VerifyIndividualResult VerifyIndividualResult;
    VerifyIndividualResult.bVerifyPassage = false;
    VerifyIndividualResult.wVerifyMask = pVerifyIndividual->wVerifyMask;
    LSTRCPYN(VerifyIndividualResult.szErrorMsg, TEXT("数据库操作异常,验证失败!"), std::size(VerifyIndividualResult.szErrorMsg));

    // 发送结果
    WORD wDataSize = CountStringBuffer(VerifyIndividualResult.szErrorMsg);
    WORD wHeadSize = sizeof(VerifyIndividualResult) - sizeof(VerifyIndividualResult.szErrorMsg);
    EmitDataBaseEngineResult(DBO_GP_VERIFY_RESULT, dwContextID, &VerifyIndividualResult, wHeadSize + wDataSize);

    return false;
  }

  return true;
}

// 游客登录
bool CDataBaseEngineSink::OnRequestLogonVisitor(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_LogonVisitor));
    if (wDataSize != sizeof(DBR_GP_LogonVisitor))
      return false;

    // 请求处理
    DBR_GP_LogonVisitor* pVisitorLogon = (DBR_GP_LogonVisitor*)pData;

    // 执行判断
    tagBindParameter* pBindParameter = (tagBindParameter*)pVisitorLogon->pBindParameter;
    if (pBindParameter->dwSocketID != dwContextID)
      return true;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pVisitorLogon->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@cbPlatformID"), pVisitorLogon->cbPlatformID);
    m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_AccountsDBAide.AddParameter(TEXT("@strMachineID"), pVisitorLogon->szMachineID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_EfficacyLogonVisitor"), true);

    // 结果处理
    StringT DBVarValue;
    m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
    OnLogonDisposeResult(dwContextID, lResultCode, DBVarValue.c_str(), 0, false);

    return true;
  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);

    // 错误处理
    OnLogonDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试或选择另一服务器登录！"), 0, false);

    return false;
  }
  return true;
}

// I D 登录
bool CDataBaseEngineSink::OnMobileLogonGameID(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_MB_LogonGameID));
    if (wDataSize != sizeof(DBR_MB_LogonGameID))
      return false;

    // 执行查询
    DBR_MB_LogonGameID* pLogonGameID = (DBR_MB_LogonGameID*)pData;

    // 变量定义
    tagBindParameter* pBindParameter = (tagBindParameter*)pLogonGameID->pBindParameter;
    if (pBindParameter->dwSocketID != dwContextID)
      return true;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pLogonGameID->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwGameID"), pLogonGameID->dwGameID);
    m_AccountsDBAide.AddParameter(TEXT("@strPassword"), pLogonGameID->szPassword);
    m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_AccountsDBAide.AddParameter(TEXT("@strMachineID"), pLogonGameID->szMachineID);
    m_AccountsDBAide.AddParameter(TEXT("@strMobilePhone"), pLogonGameID->szMobilePhone);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_MB_EfficacyGameID"), true);

    // 结果处理
    StringT DBVarValue;
    m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
    OnLogonDisposeResult(dwContextID, lResultCode, DBVarValue.c_str(), 0, true);

    return true;
  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 错误处理
    OnLogonDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试或选择另一服务器登录！"), 0, true);

    return false;
  }

  return true;
}

// 帐号登录
bool CDataBaseEngineSink::OnMobileLogonAccounts(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_MB_LogonAccounts));
    if (wDataSize != sizeof(DBR_MB_LogonAccounts))
      return false;

    // 请求处理
    DBR_MB_LogonAccounts* pLogonAccounts = (DBR_MB_LogonAccounts*)pData;

    // 执行判断
    tagBindParameter* pBindParameter = (tagBindParameter*)pLogonAccounts->pBindParameter;
    if (pBindParameter->dwSocketID != dwContextID)
      return true;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pLogonAccounts->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@strAccounts"), pLogonAccounts->szAccounts);
    m_AccountsDBAide.AddParameter(TEXT("@strPassword"), pLogonAccounts->szPassword);
    m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_AccountsDBAide.AddParameter(TEXT("@strMachineID"), pLogonAccounts->szMachineID);
    m_AccountsDBAide.AddParameter(TEXT("@strMobilePhone"), pLogonAccounts->szMobilePhone);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_MB_EfficacyAccounts"), true);

    // 结果处理
    StringT DBVarValue;
    m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
    OnLogonDisposeResult(dwContextID, lResultCode, DBVarValue.c_str(), 0, true);

    return true;
  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 错误处理
    OnLogonDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试或选择另一服务器登录！"), 0, true);

    return false;
  }

  return true;
}

// 其他登录
bool CDataBaseEngineSink::OnMobileLogonOtherPlatform(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_MB_LogonOtherPlatform));
    if (wDataSize != sizeof(DBR_MB_LogonOtherPlatform))
      return false;

    // 请求处理
    DBR_MB_LogonOtherPlatform* pLogonOtherPlatform = (DBR_MB_LogonOtherPlatform*)pData;

    // 执行判断
    tagBindParameter* pBindParameter = (tagBindParameter*)pLogonOtherPlatform->pBindParameter;
    if (pBindParameter->dwSocketID != dwContextID)
      return true;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pLogonOtherPlatform->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@cbPlatformID"), pLogonOtherPlatform->cbPlatformID);
    m_AccountsDBAide.AddParameter(TEXT("@strUserUin"), pLogonOtherPlatform->szUserUin);
    m_AccountsDBAide.AddParameter(TEXT("@strNickName"), pLogonOtherPlatform->szNickName);
    m_AccountsDBAide.AddParameter(TEXT("@strCompellation"), pLogonOtherPlatform->szCompellation);
    m_AccountsDBAide.AddParameter(TEXT("@cbGender"), pLogonOtherPlatform->cbGender);
    m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_AccountsDBAide.AddParameter(TEXT("@strMachineID"), pLogonOtherPlatform->szMachineID);
    m_AccountsDBAide.AddParameter(TEXT("@strMobilePhone"), pLogonOtherPlatform->szMobilePhone);
    m_AccountsDBAide.AddParameter(TEXT("@cbDeviceType"), pLogonOtherPlatform->cbDeviceType);
    m_AccountsDBAide.AddParameter(TEXT("@DeviceToken"), pLogonOtherPlatform->szDeviceToken);
    m_AccountsDBAide.AddParameter(TEXT("@strFaceUrl"), pLogonOtherPlatform->strFaceUrl);
    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_MB_EfficacyOtherPlatform"), true);

    // 结果处理
    StringT DBVarValue;
    m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
    OnLogonDisposeResult(dwContextID, lResultCode, DBVarValue.c_str(), 0, true);

    return true;
  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 错误处理
    OnLogonDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试或选择另一服务器登录！"), 0, true);

    return false;
  }

  return true;
}

// 游客登录
bool CDataBaseEngineSink::OnMobileLogonVisitor(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_MB_LogonVisitor));
    if (wDataSize != sizeof(DBR_MB_LogonVisitor))
      return false;

    // 请求处理
    DBR_MB_LogonVisitor* pVisitorLogon = (DBR_MB_LogonVisitor*)pData;

    // 执行判断
    tagBindParameter* pBindParameter = (tagBindParameter*)pVisitorLogon->pBindParameter;
    if (pBindParameter->dwSocketID != dwContextID)
      return true;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pVisitorLogon->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@cbPlatformID"), pVisitorLogon->cbPlatformID);
    m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_AccountsDBAide.AddParameter(TEXT("@strMachineID"), pVisitorLogon->szMachineID);
    m_AccountsDBAide.AddParameter(TEXT("@strMobilePhone"), pVisitorLogon->szMobilePhone);
    m_AccountsDBAide.AddParameter(TEXT("@cbDeviceType"), pVisitorLogon->cbDeviceType);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_MB_EfficacyLogonVisitor"), true);

    // 结果处理
    StringT DBVarValue;
    m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
    OnLogonDisposeResult(dwContextID, lResultCode, DBVarValue.c_str(), 0, true);

    return true;
  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 错误处理
    OnLogonDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试或选择另一服务器登录！"), 0, true);

    return false;
  }

  return true;
}

// 帐号注册
bool CDataBaseEngineSink::OnMobileRegisterAccounts(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_MB_RegisterAccounts));
    if (wDataSize != sizeof(DBR_MB_RegisterAccounts))
      return false;

    // 请求处理
    DBR_MB_RegisterAccounts* pRegisterAccounts = (DBR_MB_RegisterAccounts*)pData;

    // 执行判断
    tagBindParameter* pBindParameter = (tagBindParameter*)pRegisterAccounts->pBindParameter;
    if (pBindParameter->dwSocketID != dwContextID)
      return true;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pRegisterAccounts->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@strAccounts"), pRegisterAccounts->szAccounts);
    m_AccountsDBAide.AddParameter(TEXT("@strNickName"), pRegisterAccounts->szNickName);
    m_AccountsDBAide.AddParameter(TEXT("@dwSpreaderGameID"), pRegisterAccounts->dwSpreaderGameID);
    m_AccountsDBAide.AddParameter(TEXT("@strLogonPass"), pRegisterAccounts->szLogonPass);
    m_AccountsDBAide.AddParameter(TEXT("@wFaceID"), pRegisterAccounts->wFaceID);
    m_AccountsDBAide.AddParameter(TEXT("@cbGender"), pRegisterAccounts->cbGender);
    m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_AccountsDBAide.AddParameter(TEXT("@strMachineID"), pRegisterAccounts->szMachineID);
    m_AccountsDBAide.AddParameter(TEXT("@strMobilePhone"), pRegisterAccounts->szMobilePhone);
    m_AccountsDBAide.AddParameter(TEXT("@cbDeviceType"), pRegisterAccounts->cbDeviceType);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_MB_RegisterAccounts"), true);

    // 结果处理
    StringT DBVarValue;
    m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
    OnLogonDisposeResult(dwContextID, lResultCode, DBVarValue.c_str(), 0, true);

    return true;
  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 错误处理
    OnLogonDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试或选择另一服务器登录！"), 0, true);

    return false;
  }

  return true;
}

// 修改机器
bool CDataBaseEngineSink::OnRequestModifyMachine(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_ModifyMachine));
    if (wDataSize != sizeof(DBR_GP_ModifyMachine))
      return false;

    // 请求处理
    DBR_GP_ModifyMachine* pModifyMachine = (DBR_GP_ModifyMachine*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pModifyMachine->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pModifyMachine->dwUserID);
    m_AccountsDBAide.AddParameter(TEXT("@strPassword"), pModifyMachine->szPassword);
    m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_AccountsDBAide.AddParameter(TEXT("@strMachineID"), pModifyMachine->szMachineID);

    // 输出变量
    TCHAR szDescribe[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribe, sizeof(szDescribe), nanodbc::statement::PARAM_OUT);

    // 绑定操作
    if (pModifyMachine->cbBind == TRUE) {
      m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_MoorMachine"), false);
    } else {
      m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_UnMoorMachine"), false);
    }

    // 结果处理
    if (m_AccountsDBAide.GetReturnValue() == DB_SUCCESS) {
      // 变量定义
      DBO_GP_OperateSuccess OperateSuccess;
      ZeroMemory(&OperateSuccess, sizeof(OperateSuccess));

      // 获取信息
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

      // 构造变量
      OperateSuccess.lResultCode = m_AccountsDBModule->GetReturnValue();
      LSTRCPYN(OperateSuccess.szDescribeString, DBVarValue.c_str(), std::size(OperateSuccess.szDescribeString));

      // 发送结果
      WORD wStringSize = CountStringBuffer(OperateSuccess.szDescribeString);
      WORD wSendSize = sizeof(OperateSuccess) - sizeof(OperateSuccess.szDescribeString) + wStringSize;
      EmitDataBaseEngineResult(DBO_GP_OPERATE_SUCCESS, dwContextID, &OperateSuccess, wSendSize);
    } else {
      // 变量定义
      DBO_GP_OperateFailure OperateFailure;
      ZeroMemory(&OperateFailure, sizeof(OperateFailure));

      // 获取信息
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

      // 构造变量
      OperateFailure.lResultCode = m_AccountsDBModule->GetReturnValue();
      LSTRCPYN(OperateFailure.szDescribeString, DBVarValue.c_str(), std::size(OperateFailure.szDescribeString));

      // 发送结果
      WORD wStringSize = CountStringBuffer(OperateFailure.szDescribeString);
      WORD wSendSize = sizeof(OperateFailure) - sizeof(OperateFailure.szDescribeString) + wStringSize;
      EmitDataBaseEngineResult(DBO_GP_OPERATE_FAILURE, dwContextID, &OperateFailure, wSendSize);
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 结果处理
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试！"), false);

    return false;
  }

  return true;
}

// 修改头像
bool CDataBaseEngineSink::OnRequestModifySystemFace(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_ModifySystemFace));
    if (wDataSize != sizeof(DBR_GP_ModifySystemFace))
      return false;

    // 请求处理
    DBR_GP_ModifySystemFace* pModifySystemFace = (DBR_GP_ModifySystemFace*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pModifySystemFace->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pModifySystemFace->dwUserID);
    m_AccountsDBAide.AddParameter(TEXT("@strPassword"), pModifySystemFace->szPassword);
    m_AccountsDBAide.AddParameter(TEXT("@wFaceID"), pModifySystemFace->wFaceID);

    // 机器信息
    m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_AccountsDBAide.AddParameter(TEXT("@strMachineID"), pModifySystemFace->szMachineID);

    // 输出变量
    TCHAR szDescribe[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribe, sizeof(szDescribe), nanodbc::statement::PARAM_OUT);

    // 结果处理
    if (m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_SystemFaceInsert"), true) == DB_SUCCESS) {
      // 变量定义
      DBO_GP_UserFaceInfo UserFaceInfo;
      ZeroMemory(&UserFaceInfo, sizeof(UserFaceInfo));

      // 读取信息
      UserFaceInfo.wFaceID = m_AccountsDBAide.GetValue_WORD(TEXT("FaceID"));

      // 发送结果
      EmitDataBaseEngineResult(DBO_GP_USER_FACE_INFO, dwContextID, &UserFaceInfo, sizeof(UserFaceInfo));
    } else {
      // 获取参数
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

      // 结果处理
      OnOperateDisposeResult(dwContextID, m_AccountsDBAide.GetReturnValue(), DBVarValue.c_str(), false);
    }
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 结果处理
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试！"), false);

    return false;
  }

  return true;
}

// 修改头像
bool CDataBaseEngineSink::OnRequestModifyCustomFace(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_ModifyCustomFace));
    if (wDataSize != sizeof(DBR_GP_ModifyCustomFace))
      return false;

    // 请求处理
    DBR_GP_ModifyCustomFace* pModifyCustomFace = (DBR_GP_ModifyCustomFace*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pModifyCustomFace->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 变量定义
    BYTE* pcbCustomFace = (BYTE*)pModifyCustomFace->dwCustomFace;
    int CustomFaceSize = sizeof(pModifyCustomFace->dwCustomFace);

    // 获取对象
    ASSERT(m_AccountsDBModule.GetInterface() != nullptr);
    IDataBase* pIDataBase = m_AccountsDBModule.GetInterface();

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pModifyCustomFace->dwUserID);
    m_AccountsDBAide.AddParameter(TEXT("@strPassword"), pModifyCustomFace->szPassword);

    // 头像信息
    StringT VarChunk(pcbCustomFace, pcbCustomFace + CustomFaceSize);
    pIDataBase->AddParameter(TEXT("@dwCustomFace"), nanodbc::statement::PARAM_IN, VarChunk);

    // 机器信息
    m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_AccountsDBAide.AddParameter(TEXT("@strMachineID"), pModifyCustomFace->szMachineID);

    // 输出变量
    TCHAR szDescribe[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribe, sizeof(szDescribe), nanodbc::statement::PARAM_OUT);

    // 结果处理
    if (m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_CustomFaceInsert"), true) == DB_SUCCESS) {
      // 变量定义
      DBO_GP_UserFaceInfo UserFaceInfo;
      ZeroMemory(&UserFaceInfo, sizeof(UserFaceInfo));

      // 读取信息
      UserFaceInfo.wFaceID = INVALID_WORD;
      UserFaceInfo.dwCustomID = m_AccountsDBAide.GetValue_DWORD(TEXT("CustomID"));

      // 发送结果
      EmitDataBaseEngineResult(DBO_GP_USER_FACE_INFO, dwContextID, &UserFaceInfo, sizeof(UserFaceInfo));
    } else {
      // 获取参数
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

      // 结果处理
      OnOperateDisposeResult(dwContextID, m_AccountsDBAide.GetReturnValue(), DBVarValue.c_str(), false);
    }
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 结果处理
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试！"), false);

    return false;
  }

  return true;
}

// 修改密码
bool CDataBaseEngineSink::OnRequestModifyLogonPass(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_ModifyLogonPass));
    if (wDataSize != sizeof(DBR_GP_ModifyLogonPass))
      return false;

    // 请求处理
    DBR_GP_ModifyLogonPass* pModifyLogonPass = (DBR_GP_ModifyLogonPass*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pModifyLogonPass->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pModifyLogonPass->dwUserID);
    m_AccountsDBAide.AddParameter(TEXT("@strScrPassword"), pModifyLogonPass->szScrPassword);
    m_AccountsDBAide.AddParameter(TEXT("@strDesPassword"), pModifyLogonPass->szDesPassword);
    m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);

    // 输出变量
    TCHAR szDescribe[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribe, sizeof(szDescribe), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_ModifyLogonPassword"), false);

    // 结果处理
    StringT DBVarValue;
    m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
    OnOperateDisposeResult(dwContextID, lResultCode, DBVarValue.c_str(), false);

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 结果处理
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试！"), false);

    return false;
  }

  return true;
}

// 修改密码
bool CDataBaseEngineSink::OnRequestModifyInsurePass(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_ModifyInsurePass));
    if (wDataSize != sizeof(DBR_GP_ModifyInsurePass))
      return false;

    // 请求处理
    DBR_GP_ModifyInsurePass* pModifyInsurePass = (DBR_GP_ModifyInsurePass*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pModifyInsurePass->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pModifyInsurePass->dwUserID);
    m_AccountsDBAide.AddParameter(TEXT("@strScrPassword"), pModifyInsurePass->szScrPassword);
    m_AccountsDBAide.AddParameter(TEXT("@strDesPassword"), pModifyInsurePass->szDesPassword);
    m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);

    // 输出变量
    TCHAR szDescribe[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribe, sizeof(szDescribe), nanodbc::statement::PARAM_OUT);

    // 结果处理
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_ModifyInsurePassword"), false);

    // 结果处理
    StringT DBVarValue;
    m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
    OnOperateDisposeResult(dwContextID, lResultCode, DBVarValue.c_str(), false);

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 结果处理
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试！"), false);

    return false;
  }

  return true;
}

// 修改签名
bool CDataBaseEngineSink::OnRequestModifyUnderWrite(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_ModifyUnderWrite));
    if (wDataSize != sizeof(DBR_GP_ModifyUnderWrite))
      return false;

    // 请求处理
    DBR_GP_ModifyUnderWrite* pModifyUnderWrite = (DBR_GP_ModifyUnderWrite*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pModifyUnderWrite->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pModifyUnderWrite->dwUserID);
    m_AccountsDBAide.AddParameter(TEXT("@strPassword"), pModifyUnderWrite->szPassword);
    m_AccountsDBAide.AddParameter(TEXT("@strUnderWrite"), pModifyUnderWrite->szUnderWrite);
    m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);

    // 输出变量
    TCHAR szDescribe[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribe, sizeof(szDescribe), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_ModifyUnderWrite"), false);

    // 结果处理
    StringT DBVarValue;
    m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
    OnOperateDisposeResult(dwContextID, lResultCode, DBVarValue.c_str(), false);

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 结果处理
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试！"), false);

    return false;
  }

  return true;
}

bool CDataBaseEngineSink::OnRequestModifyRealAuth(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_RealAuth));
    if (wDataSize != sizeof(DBR_GP_RealAuth))
      return false;

    // 请求处理
    DBR_GP_RealAuth* pDataPara = (DBR_GP_RealAuth*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pDataPara->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pDataPara->dwUserID);
    m_AccountsDBAide.AddParameter(TEXT("@strPassword"), pDataPara->szPassword);
    m_AccountsDBAide.AddParameter(TEXT("@strCompellation"), pDataPara->szCompellation);
    m_AccountsDBAide.AddParameter(TEXT("@strPassPortID"), pDataPara->szPassPortID);
    m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);

    // 输出变量
    TCHAR szDescribe[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribe, sizeof(szDescribe), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_RealAuth"), true);

    if (lResultCode == 0) {
      // 结果处理
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
      OnIndividualDisposeResult(dwContextID, lResultCode, DBVarValue.c_str());
    } else {
      // 结果处理
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
      OnOperateDisposeResult(dwContextID, lResultCode, DBVarValue.c_str(), false);
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 结果处理
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试！"), false);

    return false;
  }

  return true;
}

// 修改资料
bool CDataBaseEngineSink::OnRequestModifyIndividual(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_ModifyIndividual));
    if (wDataSize != sizeof(DBR_GP_ModifyIndividual))
      return false;

    // 请求处理
    DBR_GP_ModifyIndividual* pModifyIndividual = (DBR_GP_ModifyIndividual*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pModifyIndividual->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pModifyIndividual->dwUserID);
    m_AccountsDBAide.AddParameter(TEXT("@strPassword"), pModifyIndividual->szPassword);
    m_AccountsDBAide.AddParameter(TEXT("@cbGender"), pModifyIndividual->cbGender);
    m_AccountsDBAide.AddParameter(TEXT("@strNickName"), pModifyIndividual->szNickName);
    m_AccountsDBAide.AddParameter(TEXT("@strUnderWrite"), pModifyIndividual->szUnderWrite);
    m_AccountsDBAide.AddParameter(TEXT("@strCompellation"), pModifyIndividual->szCompellation);
    m_AccountsDBAide.AddParameter(TEXT("@strPassPortID"), pModifyIndividual->szPassPortID);
    m_AccountsDBAide.AddParameter(TEXT("@strQQ"), pModifyIndividual->szQQ);
    m_AccountsDBAide.AddParameter(TEXT("@strEMail"), pModifyIndividual->szEMail);
    m_AccountsDBAide.AddParameter(TEXT("@strSeatPhone"), pModifyIndividual->szSeatPhone);
    m_AccountsDBAide.AddParameter(TEXT("@strMobilePhone"), pModifyIndividual->szMobilePhone);
    m_AccountsDBAide.AddParameter(TEXT("@strDwellingPlace"), pModifyIndividual->szDwellingPlace);
    m_AccountsDBAide.AddParameter(TEXT("@strUserNote"), pModifyIndividual->szUserNote);
    m_AccountsDBAide.AddParameter(TEXT("@strSpreader"), pModifyIndividual->szSpreader);
    m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);

    // 输出变量
    TCHAR szDescribe[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribe, sizeof(szDescribe), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_ModifyUserIndividual"), true);

    // 结果处理
    StringT DBVarValue;
    m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
    OnOperateDisposeResult(dwContextID, lResultCode, DBVarValue.c_str(), false);

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 结果处理
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试！"), false);

    return false;
  }

  return true;
}

// 开通银行
bool CDataBaseEngineSink::OnRequestUserEnableInsure(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_UserEnableInsure));
    if (wDataSize != sizeof(DBR_GP_UserEnableInsure))
      return false;

    // 请求处理
    DBR_GP_UserEnableInsure* pUserEnableInsure = (DBR_GP_UserEnableInsure*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pUserEnableInsure->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_TreasureDBAide.ResetParameter();
    m_TreasureDBAide.AddParameter(TEXT("@dwUserID"), pUserEnableInsure->dwUserID);
    m_TreasureDBAide.AddParameter(TEXT("@strLogonPass"), pUserEnableInsure->szLogonPass);
    m_TreasureDBAide.AddParameter(TEXT("@strInsurePass"), pUserEnableInsure->szInsurePass);
    m_TreasureDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_TreasureDBAide.AddParameter(TEXT("@strMachineID"), pUserEnableInsure->szMachineID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_TreasureDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_TreasureDBAide.ExecuteProcess(TEXT("GSP_GR_UserEnableInsure"), true);

    // 结果处理
    StringT DBVarValue;
    m_TreasureDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

    // 构造对象
    DBO_GP_UserInsureEnableResult UserEnableInsureResult;
    ZeroMemory(&UserEnableInsureResult, sizeof(UserEnableInsureResult));

    // 设置变量
    UserEnableInsureResult.cbInsureEnabled = (lResultCode == DB_SUCCESS) ? TRUE : FALSE;
    LSTRCPYN(UserEnableInsureResult.szDescribeString, DBVarValue.c_str(), std::size(UserEnableInsureResult.szDescribeString));

    // 发送结果
    WORD wDataSize = CountStringBuffer(UserEnableInsureResult.szDescribeString);
    WORD wHeadSize = sizeof(UserEnableInsureResult) - sizeof(UserEnableInsureResult.szDescribeString);
    EmitDataBaseEngineResult(DBO_GP_USER_INSURE_ENABLE_RESULT, dwContextID, &UserEnableInsureResult, wHeadSize + wDataSize);

    return true;
  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 构造对象
    DBO_GP_UserInsureEnableResult UserEnableInsureResult;
    ZeroMemory(&UserEnableInsureResult, sizeof(UserEnableInsureResult));

    // 设置变量
    UserEnableInsureResult.cbInsureEnabled = FALSE;
    LSTRCPYN(UserEnableInsureResult.szDescribeString, TEXT("由于数据库操作异常，请您稍后重试！"),
             std::size(UserEnableInsureResult.szDescribeString));

    // 发送结果
    WORD wDataSize = CountStringBuffer(UserEnableInsureResult.szDescribeString);
    WORD wHeadSize = sizeof(UserEnableInsureResult) - sizeof(UserEnableInsureResult.szDescribeString);
    EmitDataBaseEngineResult(DBO_GP_USER_INSURE_ENABLE_RESULT, dwContextID, &UserEnableInsureResult, wHeadSize + wDataSize);

    return false;
  }
}

// 存入游戏币
bool CDataBaseEngineSink::OnRequestUserSaveScore(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_UserSaveScore));
    if (wDataSize != sizeof(DBR_GP_UserSaveScore))
      return false;

    // 请求处理
    DBR_GP_UserSaveScore* pUserSaveScore = (DBR_GP_UserSaveScore*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pUserSaveScore->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_TreasureDBAide.ResetParameter();
    m_TreasureDBAide.AddParameter(TEXT("@dwUserID"), pUserSaveScore->dwUserID);
    m_TreasureDBAide.AddParameter(TEXT("@lSaveScore"), pUserSaveScore->lSaveScore);
    m_TreasureDBAide.AddParameter(TEXT("@wKindID"), 0L);
    m_TreasureDBAide.AddParameter(TEXT("@wServerID"), 0L);
    m_TreasureDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_TreasureDBAide.AddParameter(TEXT("@strMachineID"), pUserSaveScore->szMachineID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_TreasureDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_TreasureDBAide.ExecuteProcess(TEXT("GSP_GR_UserSaveScore"), true);

    // 结果处理
    StringT DBVarValue;
    m_TreasureDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
    OnInsureDisposeResult(dwContextID, lResultCode, DBVarValue.c_str(), false);

    return true;
  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 错误处理
    OnInsureDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试！"), false);

    return false;
  }

  return true;
}

// 提取游戏币
bool CDataBaseEngineSink::OnRequestUserTakeScore(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_UserTakeScore));
    if (wDataSize != sizeof(DBR_GP_UserTakeScore))
      return false;

    // 请求处理
    DBR_GP_UserTakeScore* pUserTakeScore = (DBR_GP_UserTakeScore*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pUserTakeScore->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_TreasureDBAide.ResetParameter();
    m_TreasureDBAide.AddParameter(TEXT("@dwUserID"), pUserTakeScore->dwUserID);
    m_TreasureDBAide.AddParameter(TEXT("@lTakeScore"), pUserTakeScore->lTakeScore);
    m_TreasureDBAide.AddParameter(TEXT("@strPassword"), pUserTakeScore->szPassword);
    m_TreasureDBAide.AddParameter(TEXT("@wKindID"), 0L);
    m_TreasureDBAide.AddParameter(TEXT("@wServerID"), 0L);
    m_TreasureDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_TreasureDBAide.AddParameter(TEXT("@strMachineID"), pUserTakeScore->szMachineID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_TreasureDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_TreasureDBAide.ExecuteProcess(TEXT("GSP_GR_UserTakeScore"), true);

    // 结果处理
    StringT DBVarValue;
    m_TreasureDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
    OnInsureDisposeResult(dwContextID, lResultCode, DBVarValue.c_str(), false);

    return true;
  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 错误处理
    OnInsureDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试！"), false);

    return false;
  }

  return true;
}

// 转帐游戏币
bool CDataBaseEngineSink::OnRequestUserTransferScore(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_UserTransferScore));
    if (wDataSize != sizeof(DBR_GP_UserTransferScore))
      return false;

    // 请求处理
    DBR_GP_UserTransferScore* pUserTransferScore = (DBR_GP_UserTransferScore*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pUserTransferScore->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_TreasureDBAide.ResetParameter();
    m_TreasureDBAide.AddParameter(TEXT("@dwUserID"), pUserTransferScore->dwUserID);
    m_TreasureDBAide.AddParameter(TEXT("@lTransferScore"), pUserTransferScore->lTransferScore);
    m_TreasureDBAide.AddParameter(TEXT("@strPassword"), pUserTransferScore->szPassword);
    m_TreasureDBAide.AddParameter(TEXT("@dwGameID"), pUserTransferScore->dwGameID);
    m_TreasureDBAide.AddParameter(TEXT("@strTransRemark"), pUserTransferScore->szTransRemark);
    m_TreasureDBAide.AddParameter(TEXT("@wKindID"), 0L);
    m_TreasureDBAide.AddParameter(TEXT("@wServerID"), 0L);
    m_TreasureDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_TreasureDBAide.AddParameter(TEXT("@strMachineID"), pUserTransferScore->szMachineID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_TreasureDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_TreasureDBAide.ExecuteProcess(TEXT("GSP_GR_UserTransferScore"), true);

    // 结果处理
    StringT DBVarValue;
    m_TreasureDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
    OnInsureDisposeResult(dwContextID, lResultCode, DBVarValue.c_str(), false);

    return true;
  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 错误处理
    OnInsureDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试！"), false);

    return false;
  }

  return true;
}

// 请求配置
bool CDataBaseEngineSink::OnRequestLotteryConfigReq(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_LotteryConfigReq));
    if (wDataSize != sizeof(DBR_GP_LotteryConfigReq))
      return false;

    // 请求处理
    DBR_GP_LotteryConfigReq* pLotteryConfigReq = (DBR_GP_LotteryConfigReq*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pLotteryConfigReq->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@wKindID"), pLotteryConfigReq->wKindID);

    // 抽奖配置
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_LoadLotteryConfig"), true);
    ASSERT(lResultCode == DB_SUCCESS);
    if (lResultCode == DB_SUCCESS) {
      // 构造结果
      DBO_GP_LotteryConfig LotteryConfig;
      ZeroMemory(&LotteryConfig, sizeof(LotteryConfig));

      // 变量定义
      WORD wItemCount = 0;
      while (!m_PlatformDBModule->IsRecordsetEnd()) {
        // 读取数据
        LotteryConfig.LotteryItem[wItemCount].cbItemIndex = m_PlatformDBAide.GetValue_BYTE(TEXT("ItemIndex"));
        LotteryConfig.LotteryItem[wItemCount].cbItemType = m_PlatformDBAide.GetValue_BYTE(TEXT("ItemType"));
        LotteryConfig.LotteryItem[wItemCount].lItemQuota = m_PlatformDBAide.GetValue_WORD(TEXT("ItemQuota"));
        wItemCount++;

        // 移动记录
        m_PlatformDBModule->MoveToNext();
      }

      LotteryConfig.wLotteryCount = wItemCount;

      // 投递结果
      EmitDataBaseEngineResult(DBO_GP_LOTTERY_CONFIG, dwContextID, &LotteryConfig, sizeof(LotteryConfig));
    } else {
      CLogger::Error(TEXT("抽奖配置加载失败！"));
    }

    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@wKindID"), pLotteryConfigReq->wKindID);
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pLotteryConfigReq->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@strLogonPass"), pLotteryConfigReq->szLogonPass);
    m_PlatformDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 抽奖信息
    lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_LotteryUserInfo"), true);
    if (lResultCode == DB_SUCCESS) {
      // 构造结果
      DBO_GP_LotteryUserInfo LotteryUserInfo;
      ZeroMemory(&LotteryUserInfo, sizeof(LotteryUserInfo));

      LotteryUserInfo.wKindID = pLotteryConfigReq->wKindID;
      LotteryUserInfo.dwUserID = pLotteryConfigReq->dwUserID;
      LotteryUserInfo.cbFreeCount = m_PlatformDBAide.GetValue_BYTE(TEXT("FreeCount"));
      LotteryUserInfo.cbAlreadyCount = m_PlatformDBAide.GetValue_BYTE(TEXT("AlreadyCount"));
      LotteryUserInfo.lChargeFee = m_PlatformDBAide.GetValue_LONGLONG(TEXT("ChargeFee"));

      // 投递结果
      EmitDataBaseEngineResult(DBO_GP_LOTTERY_USER_INFO, dwContextID, &LotteryUserInfo, sizeof(LotteryUserInfo));
    } else {
      // 变量定义
      DBO_GP_OperateFailure OperateFailure;
      ZeroMemory(&OperateFailure, sizeof(OperateFailure));

      // 获取信息
      StringT DBVarValue;
      m_PlatformDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

      // 构造变量
      OperateFailure.lResultCode = m_PlatformDBModule->GetReturnValue();
      LSTRCPYN(OperateFailure.szDescribeString, DBVarValue.c_str(), std::size(OperateFailure.szDescribeString));

      // 发送结果
      WORD wStringSize = CountStringBuffer(OperateFailure.szDescribeString);
      WORD wSendSize = sizeof(OperateFailure) - sizeof(OperateFailure.szDescribeString) + wStringSize;
      EmitDataBaseEngineResult(DBO_GP_OPERATE_FAILURE, dwContextID, &OperateFailure, wSendSize);
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 结果处理
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试！"), false);

    return false;
  }

  return true;
}

// 抽奖开始
bool CDataBaseEngineSink::OnRequestLotteryStart(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_LotteryStart));
    if (wDataSize != sizeof(DBR_GP_LotteryStart))
      return false;

    // 请求处理
    DBR_GP_LotteryStart* pLotteryStart = (DBR_GP_LotteryStart*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pLotteryStart->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@wKindID"), pLotteryStart->wKindID);
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pLotteryStart->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@strLogonPass"), pLotteryStart->szLogonPass);
    m_PlatformDBAide.AddParameter(TEXT("@strMachineID"), pLotteryStart->szMachineID);
    m_PlatformDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_LotteryStart"), true);

    if (lResultCode == DB_SUCCESS) {
      // 构造结果
      DBO_GP_LotteryResult LotteryResult;
      ZeroMemory(&LotteryResult, sizeof(LotteryResult));

      LotteryResult.wKindID = pLotteryStart->wKindID;
      LotteryResult.dwUserID = pLotteryStart->dwUserID;
      LotteryResult.bWined = (m_PlatformDBAide.GetValue_BYTE(TEXT("Wined")) == TRUE);
      LotteryResult.lUserScore = m_PlatformDBAide.GetValue_LONGLONG(TEXT("UserScore"));
      LotteryResult.dUserBeans = m_PlatformDBAide.GetValue_DOUBLE(TEXT("UserBeans"));
      LotteryResult.LotteryItem.cbItemIndex = m_PlatformDBAide.GetValue_BYTE(TEXT("ItemIndex"));
      LotteryResult.LotteryItem.cbItemType = m_PlatformDBAide.GetValue_BYTE(TEXT("ItemType"));
      LotteryResult.LotteryItem.lItemQuota = m_PlatformDBAide.GetValue_LONGLONG(TEXT("ItemQuota"));

      // 投递结果
      EmitDataBaseEngineResult(DBO_GP_LOTTERY_RESULT, dwContextID, &LotteryResult, sizeof(LotteryResult));
    } else {
      // 变量定义
      DBO_GP_OperateFailure OperateFailure;
      ZeroMemory(&OperateFailure, sizeof(OperateFailure));

      // 获取信息
      StringT DBVarValue;
      m_PlatformDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

      // 构造变量
      OperateFailure.lResultCode = m_PlatformDBModule->GetReturnValue();
      LSTRCPYN(OperateFailure.szDescribeString, DBVarValue.c_str(), std::size(OperateFailure.szDescribeString));

      // 发送结果
      WORD wStringSize = CountStringBuffer(OperateFailure.szDescribeString);
      WORD wSendSize = sizeof(OperateFailure) - sizeof(OperateFailure.szDescribeString) + wStringSize;
      EmitDataBaseEngineResult(DBO_GP_OPERATE_FAILURE, dwContextID, &OperateFailure, wSendSize);
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 结果处理
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试！"), false);

    return false;
  }

  return true;
}

// 游戏数据
bool CDataBaseEngineSink::OnRequestQueryUserGameData(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_QueryUserGameData));
    if (wDataSize != sizeof(DBR_GP_QueryUserGameData))
      return false;

    // 请求处理
    DBR_GP_QueryUserGameData* pQueryUserGameData = (DBR_GP_QueryUserGameData*)pData;

    // 构造参数
    m_TreasureDBAide.ResetParameter();
    m_TreasureDBAide.AddParameter(TEXT("@wKindID"), pQueryUserGameData->wKindID);
    m_TreasureDBAide.AddParameter(TEXT("@dwUserID"), pQueryUserGameData->dwUserID);
    m_TreasureDBAide.AddParameter(TEXT("@strDynamicPass"), pQueryUserGameData->szDynamicPass);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_TreasureDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_TreasureDBAide.ExecuteProcess(TEXT("GSP_GR_QueryUserGameData"), true);

    if (lResultCode == DB_SUCCESS) {
      // 构造结果
      DBO_GP_QueryUserGameData QueryUserGameData;
      ZeroMemory(&QueryUserGameData, sizeof(QueryUserGameData));

      QueryUserGameData.wKindID = pQueryUserGameData->wKindID;
      QueryUserGameData.dwUserID = pQueryUserGameData->dwUserID;
      m_TreasureDBAide.GetValue_String(TEXT("UserGameData"), QueryUserGameData.szUserGameData, std::size(QueryUserGameData.szUserGameData));

      // 投递结果
      EmitDataBaseEngineResult(DBO_GP_QUERY_USER_GAME_DATA, dwContextID, &QueryUserGameData, sizeof(QueryUserGameData));
    } else {
      // 变量定义
      DBO_GP_OperateFailure OperateFailure;
      ZeroMemory(&OperateFailure, sizeof(OperateFailure));

      // 获取信息
      StringT DBVarValue;
      m_TreasureDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

      // 构造变量
      OperateFailure.lResultCode = m_TreasureDBModule->GetReturnValue();
      LSTRCPYN(OperateFailure.szDescribeString, DBVarValue.c_str(), std::size(OperateFailure.szDescribeString));

      // 发送结果
      WORD wStringSize = CountStringBuffer(OperateFailure.szDescribeString);
      WORD wSendSize = sizeof(OperateFailure) - sizeof(OperateFailure.szDescribeString) + wStringSize;
      EmitDataBaseEngineResult(DBO_GP_OPERATE_FAILURE, dwContextID, &OperateFailure, wSendSize);
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 结果处理
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试！"), false);

    return false;
  }

  return true;
}

// 约战房间配置
bool CDataBaseEngineSink::OnRequestGetPersonalParameter(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_MB_GetPersonalParameter));
    if (wDataSize != sizeof(DBR_MB_GetPersonalParameter))
      return false;

    // 请求处理
    DBR_MB_GetPersonalParameter* pGetPersonalParameter = (DBR_MB_GetPersonalParameter*)pData;

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwKindID"), pGetPersonalParameter->dwKindID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    LONG lResultCode = 0;
    lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GS_LoadPersonalRoomParameter"), true);

    if (lResultCode == DB_SUCCESS) {
      tagPersonalRoomOption pPersonalRoomOption;
      ZeroMemory(&pPersonalRoomOption, sizeof(tagPersonalRoomOption));

      // 读取约战房房间信息
      pPersonalRoomOption.lPersonalRoomTax = m_PlatformDBAide.GetValue_LONGLONG(TEXT("PersonalRoomTax"));
      // pPersonalRoomOption.lFeeCardOrBeanCount =  m_PlatformDBAide.GetValue_LONGLONG(TEXT("DiamondCount"));

      // pPersonalRoomOption.cbCardOrBean=m_PlatformDBAide.GetValue_BYTE(TEXT("CardOrBean"));
      pPersonalRoomOption.cbIsJoinGame = m_PlatformDBAide.GetValue_BYTE(TEXT("IsJoinGame"));
      pPersonalRoomOption.cbMinPeople = m_PlatformDBAide.GetValue_BYTE(TEXT("MinPeople"));
      pPersonalRoomOption.cbMaxPeople = m_PlatformDBAide.GetValue_BYTE(TEXT("MaxPeople"));
      pPersonalRoomOption.lMaxCellScore = m_PlatformDBAide.GetValue_LONGLONG(TEXT("MaxCellScore"));

      pPersonalRoomOption.wCanCreateCount = m_PlatformDBAide.GetValue_WORD(TEXT("CanCreateCount"));
      pPersonalRoomOption.dwPlayTimeLimit = m_PlatformDBAide.GetValue_DWORD(TEXT("PlayTimeLimit"));
      pPersonalRoomOption.dwPlayTurnCount = m_PlatformDBAide.GetValue_DWORD(TEXT("PlayTurnCount"));

      pPersonalRoomOption.wBeginFreeTime = m_PlatformDBAide.GetValue_WORD(TEXT("BeginFreeTime"));
      pPersonalRoomOption.wEndFreeTime = m_PlatformDBAide.GetValue_WORD(TEXT("EndFreeTime"));

      EmitDataBaseEngineResult(DBO_MB_PERSONAL_PARAMETER, dwContextID, &pPersonalRoomOption, sizeof(tagPersonalRoomOption));
    } else {
      // 错误信息
      CLogger::Error(TEXT("数据库异常,请稍后再试！"));

      // 操作结果
      OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("数据库异常,请稍后再试！"), false, false);
    }

    // 请求底分配置
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwKindID"), pGetPersonalParameter->dwKindID);

    lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_MB_GetPersonalCellScore"), true);

    if (lResultCode == DB_SUCCESS) {
      // 变量定义
      int nCount = 0;
      tagPersonalCellScore PersonalCellScore;
      ZeroMemory(&PersonalCellScore, sizeof(PersonalCellScore));

      while (!m_PlatformDBModule->IsRecordsetEnd()) {
        if (nCount < CELLSCORE_COUNT) {
          PersonalCellScore.nCellScore[nCount++] = m_PlatformDBAide.GetValue_INT(TEXT("CellScore"));
        }

        // 移动记录
        m_PlatformDBModule->MoveToNext();
      }

      EmitDataBaseEngineResult(DBO_MB_PERSONAL_CELL_SCORE_LIST, dwContextID, &PersonalCellScore, sizeof(PersonalCellScore));

    } else {
      // 错误信息
      CLogger::Error(TEXT("GSP_MB_GetPersonalCellScore 执行错误！"));

      // 操作结果
      OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("数据库异常,请稍后再试！"), false, false);
    }

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwKindID"), pGetPersonalParameter->dwKindID);

    // 输出参数
    // TCHAR szDescribeString[128]=TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_MB_GetPersonalFeeParameter"), true);

    if (lResultCode == DB_SUCCESS) {
      // 变量定义
      WORD dwPaketSize = 0;
      BYTE cbBuffer[MAX_ASYNCHRONISM_DATA];
      tagPersonalTableFeeList* pPersonalTableParameter = nullptr;
      ZeroMemory(cbBuffer, sizeof(cbBuffer));

      while (!m_PlatformDBModule->IsRecordsetEnd()) {
        // 发送信息
        if ((dwPaketSize + sizeof(tagPersonalTableFeeList)) > (sizeof(cbBuffer) - sizeof(NTY_DataBaseEvent))) {
          EmitDataBaseEngineResult(DBO_MB_PERSONAL_FEE_LIST, dwContextID, cbBuffer, dwPaketSize);
          ZeroMemory(cbBuffer, sizeof(cbBuffer));
          dwPaketSize = 0;
        }

        // 读取信息
        pPersonalTableParameter = (tagPersonalTableFeeList*)(cbBuffer + dwPaketSize);
        pPersonalTableParameter->dwDrawCountLimit = m_PlatformDBAide.GetValue_DWORD(TEXT("DrawCountLimit"));
        pPersonalTableParameter->dwDrawTimeLimit = m_PlatformDBAide.GetValue_DWORD(TEXT("DrawTimeLimit"));
        pPersonalTableParameter->lFeeScore = m_PlatformDBAide.GetValue_LONGLONG(TEXT("TableFee"));
        pPersonalTableParameter->wAAPayFee = m_PlatformDBAide.GetValue_WORD(TEXT("AAPayFee"));
        pPersonalTableParameter->lIniScore = m_PlatformDBAide.GetValue_LONGLONG(TEXT("IniScore"));
        pPersonalTableParameter->cbGameMode = m_PlatformDBAide.GetValue_BYTE(TEXT("GameMode"));

        // 设置位移
        dwPaketSize += sizeof(tagPersonalTableFeeList);

        // 移动记录
        m_PlatformDBModule->MoveToNext();
      }

      if (dwPaketSize > 0) {
        EmitDataBaseEngineResult(DBO_MB_PERSONAL_FEE_LIST, dwContextID, cbBuffer, dwPaketSize);
      }
    } else {
      // 错误信息
      CLogger::Error(TEXT("获取私人房费用列表出错！"));

      // 操作结果
      OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("数据库异常,请稍后再试！"), false, false);
    }

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwKindID"), pGetPersonalParameter->dwKindID);

    // 输出参数
    // TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_MB_GetPersonalRule"), true);

    if (lResultCode == DB_SUCCESS) {
      tagGetPersonalRule GetPersonalRule;
      ZeroMemory(&GetPersonalRule, sizeof(tagGetPersonalRule));

      TCHAR szPersonalRule[std::size(GetPersonalRule.cbPersonalRule) * 2 + 1] = {};
      if (!m_PlatformDBModule->IsRecordsetEnd()) {
        m_PlatformDBAide.GetValue_String(TEXT("PersonalRule"), szPersonalRule, std::size(szPersonalRule));
        // 扩展配置
        if (szPersonalRule[0] != 0 && szPersonalRule[1] == '1') {
          INT nPersonalRuleSize = StrLenT(szPersonalRule) / 2;

          // 转换字符
          for (INT i = 0; i < nPersonalRuleSize; i++) {
            // 获取字符
            TCHAR cbChar1 = szPersonalRule[i * 2];
            TCHAR cbChar2 = szPersonalRule[i * 2 + 1];

            // 效验字符
            ASSERT((cbChar1 >= TEXT('0')) && (cbChar1 <= TEXT('9')) || (cbChar1 >= TEXT('A')) && (cbChar1 <= TEXT('F')));
            ASSERT((cbChar2 >= TEXT('0')) && (cbChar2 <= TEXT('9')) || (cbChar2 >= TEXT('A')) && (cbChar2 <= TEXT('F')));

            // 生成结果
            if ((cbChar2 >= TEXT('0')) && (cbChar2 <= TEXT('9')))
              GetPersonalRule.cbPersonalRule[i] += (cbChar2 - TEXT('0'));
            if ((cbChar2 >= TEXT('A')) && (cbChar2 <= TEXT('F')))
              GetPersonalRule.cbPersonalRule[i] += (cbChar2 - TEXT('A') + 0x0A);

            // 生成结果
            if ((cbChar1 >= TEXT('0')) && (cbChar1 <= TEXT('9')))
              GetPersonalRule.cbPersonalRule[i] += (cbChar1 - TEXT('0')) * 0x10;
            if ((cbChar1 >= TEXT('A')) && (cbChar1 <= TEXT('F')))
              GetPersonalRule.cbPersonalRule[i] += (cbChar1 - TEXT('A') + 0x0A) * 0x10;
          }

          EmitDataBaseEngineResult(DBO_MB_GET_PERSONAL_RULE, dwContextID, &GetPersonalRule, sizeof(tagGetPersonalRule));
        }
      }
    }

    // 获取结束
    EmitDataBaseEngineResult(DBO_MB_GET_PARAMETER_END, dwContextID, nullptr, 0);

  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 操作结果
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("数据库异常,请稍后再试！"), false, false);
  }

  return true;
}

int64_t TimeDiff(SYSTEMTIME left, SYSTEMTIME right) {
  std::tm tmLeft = {};
  tmLeft.tm_year = left.wYear - 1900;
  tmLeft.tm_mon = left.wMonth - 1;
  tmLeft.tm_mday = left.wDay;
  // tmLeft.tm_wday = left.wDayOfWeek;
  std::tm tmRight = {};
  tmRight.tm_year = right.wYear - 1900;
  tmRight.tm_mon = right.wMonth - 1;
  tmRight.tm_mday = right.wDay;
  // tmRight.tm_wday = right.wDayOfWeek;

  std::time_t gmtLeft = std::mktime(&tmLeft);
  std::time_t gmtRight = std::mktime(&tmRight);

  return (gmtLeft - gmtRight) * 1000 + (left.wMilliseconds - right.wMilliseconds); // 此处返回毫秒
}

// 两个字符转换成一个字符，长度为原来的1/2
void Hex2Char(const char* szHex, unsigned char& rch) {
  rch = 0;
  int i;
  for (i = 0; i < 2; i++) {
    if (i == 0) {
      if (*(szHex + i) >= '0' && *(szHex + i) <= '9')
        rch += (*(szHex + i) - '0') * 16;
      else if (*(szHex + i) >= 'a' && *(szHex + i) <= 'f')
        rch += (*(szHex + i) - 'a' + 10) * 16;
      else if (*(szHex + i) >= 'A' && *(szHex + i) <= 'F')
        rch += (*(szHex + i) - 'A' + 10) * 16;
    } else {
      if (*(szHex + i) >= '0' && *(szHex + i) <= '9')
        rch += (*(szHex + i) - '0');
      else if (*(szHex + i) >= 'a' && *(szHex + i) <= 'f')
        rch += *(szHex + i) - 'a' + 10;
      else if (*(szHex + i) >= 'A' && *(szHex + i) <= 'F')
        rch += *(szHex + i) - 'A' + 10;
    }
  }
}

/// 十六进制char* 转 Binary char*函数
void HexStr2CharStr(const char* pszHexStr, int iSize, BYTE* pucCharStr) {
  int i;
  unsigned char ch;
  if (iSize % 2 != 0)
    return;
  for (i = 0; i < iSize / 2; i++) {
    Hex2Char(pszHexStr + 2 * i, ch);
    pucCharStr[i] = ch;
  }
}

bool CDataBaseEngineSink::OnRequestqueryPersonalRoomInfo(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(CMD_CS_C_HostCreatRoomInfo));
    if (wDataSize != sizeof(CMD_CS_C_HostCreatRoomInfo))
      return false;

    // 请求处理

    CMD_CS_C_HostCreatRoomInfo* pGetParameter = (CMD_CS_C_HostCreatRoomInfo*)pData;

    DBO_MB_PersonalRoomInfoList PersonalRoomInfoList;
    memset(&PersonalRoomInfoList, 0, sizeof(DBO_MB_PersonalRoomInfoList));

    // 玩家id
    PersonalRoomInfoList.dwUserID = pGetParameter->HostCreatRoomInfo.dwUserID;

    DWORD dwPersonalRoomID = 0;
    int iCount = 0;
    for (int i = 0; i < MAX_CREATE_PERSONAL_ROOM; i++) {
      if (pGetParameter->HostCreatRoomInfo.hRoomCreateInfo[i].dwPersonalRoomID == 0) {
        continue;
      }
      iCount = i;
      dwPersonalRoomID = pGetParameter->HostCreatRoomInfo.hRoomCreateInfo[i].dwPersonalRoomID;

      // 构造参数
      m_TreasureDBAide.ResetParameter();
      m_TreasureDBAide.AddParameter(TEXT("@dwPersonalRoomID"), dwPersonalRoomID);
      m_TreasureDBAide.AddParameter(TEXT("@dwRoomHostID"), pGetParameter->HostCreatRoomInfo.dwUserID);

      // 输出参数
      TCHAR szDescribeString[128] = TEXT("");
      m_TreasureDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

      // 执行查询
      LONG lResultCode = m_TreasureDBAide.ExecuteProcess(TEXT("GSP_GS_QueryPersonalRoomInfo"), true);
      if (DB_SUCCESS == lResultCode) {
        TCHAR szInfo[520] = {0};

        PersonalRoomInfoList.PersonalRoomInfo[i].dwPersonalRoomID = pGetParameter->HostCreatRoomInfo.hRoomCreateInfo[i].dwPersonalRoomID;
        PersonalRoomInfoList.PersonalRoomInfo[i].dwGameKindID = m_TreasureDBAide.GetValue_INT(TEXT("wKindID"));
        PersonalRoomInfoList.PersonalRoomInfo[i].dwPlayTimeLimit = m_TreasureDBAide.GetValue_DWORD(TEXT("dwPlayTimeLimit"));
        PersonalRoomInfoList.PersonalRoomInfo[i].cbPlayTurnCount = m_TreasureDBAide.GetValue_BYTE(TEXT("dwPlayTurnCount"));
        PersonalRoomInfoList.PersonalRoomInfo[i].cbIsDisssumRoom = m_TreasureDBAide.GetValue_BYTE(TEXT("RoomStatus"));
        m_TreasureDBAide.GetValue_SystemTime(TEXT("sysCreateTime"), PersonalRoomInfoList.PersonalRoomInfo[i].sysCreateTime);
        m_TreasureDBAide.GetValue_SystemTime(TEXT("sysDissumeTime"), PersonalRoomInfoList.PersonalRoomInfo[i].sysDissumeTime);

        PersonalRoomInfoList.PersonalRoomInfo[i].cbPayMode = m_TreasureDBAide.GetValue_BYTE(TEXT("PayMode"));
        PersonalRoomInfoList.PersonalRoomInfo[i].cbNeedRoomCard = m_TreasureDBAide.GetValue_BYTE(TEXT("NeedRoomCard"));
        PersonalRoomInfoList.PersonalRoomInfo[i].cbPlayerCount = m_TreasureDBAide.GetValue_BYTE(TEXT("JoinGamePeopleCount"));
        PersonalRoomInfoList.PersonalRoomInfo[i].cbGameMode = m_TreasureDBAide.GetValue_BYTE(TEXT("GameMode"));

        PersonalRoomInfoList.PersonalRoomInfo[i].wBeginFreeTime = pGetParameter->HostCreatRoomInfo.hRoomCreateInfo[i].wBeginFreeTime;
        PersonalRoomInfoList.PersonalRoomInfo[i].wEndFreeTime = pGetParameter->HostCreatRoomInfo.hRoomCreateInfo[i].wEndFreeTime;
        PersonalRoomInfoList.PersonalRoomInfo[i].wCurSitCount = pGetParameter->HostCreatRoomInfo.hRoomCreateInfo[i].wCurSitCount;
        // 读取二进制数据并转化为相应数据结构
        BYTE szTempBinary[1858] = {0};
        TCHAR szTempChar[3712] = {0};

        m_TreasureDBAide.GetValue_String(TEXT("strRoomScoreInfo"), szTempChar, std::size(szTempChar));
        std::string strTempChar = ToSimpleUtf8(szTempChar);
        HexStr2CharStr(strTempChar.c_str(), strTempChar.length(), szTempBinary);

        for (int j = 0; j < PERSONAL_ROOM_CHAIR; j++) {
          memcpy(&PersonalRoomInfoList.PersonalRoomInfo[i].PersonalUserScoreInfo[j], szTempBinary + j * sizeof(tagPersonalUserScoreInfo),
                 sizeof(tagPersonalUserScoreInfo));
        }
      }
    }

    // 创建时间排序
    for (int i = 0; i <= iCount; i++) {
      for (int j = i + 1; j <= iCount; j++) {
        if (TimeDiff(PersonalRoomInfoList.PersonalRoomInfo[i].sysCreateTime, PersonalRoomInfoList.PersonalRoomInfo[j].sysCreateTime) > 0) {
          tagPersonalRoomInfo temp;
          memset(&temp, 0, sizeof(tagPersonalRoomInfo));
          memcpy(&temp, &PersonalRoomInfoList.PersonalRoomInfo[i], sizeof(tagPersonalRoomInfo));
          memcpy(&PersonalRoomInfoList.PersonalRoomInfo[i], &PersonalRoomInfoList.PersonalRoomInfo[j], sizeof(tagPersonalRoomInfo));
          memcpy(&PersonalRoomInfoList.PersonalRoomInfo[j], &temp, sizeof(tagPersonalRoomInfo));
        }
      }
    }

    EmitDataBaseEngineResult(DBO_MB_PERSONAL_ROOM_LIST, dwContextID, &PersonalRoomInfoList, sizeof(PersonalRoomInfoList));

    return true;
  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 操作结果
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("数据库异常,请稍后再试！"), false, false);

    return false;
  }
}

// 请求私人房间信息
bool CDataBaseEngineSink::OnRequestQueryPersonalRule(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  return 0;
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_MB_GetPersonalRule));
    if (wDataSize != sizeof(DBR_MB_GetPersonalRule))
      return false;

    // 请求处理
    DBR_MB_GetPersonalRule* pGetPersonalRule = (DBR_MB_GetPersonalRule*)pData;

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwServerID"), pGetPersonalRule->dwServerID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_MB_GetPersonalRule"), true);

    if (lResultCode == DB_SUCCESS) {
      tagGetPersonalRule GetPersonalRule;
      ZeroMemory(&GetPersonalRule, sizeof(tagGetPersonalRule));

      TCHAR szPersonalRule[std::size(GetPersonalRule.cbPersonalRule) * 2 + 1] = {};
      m_PlatformDBAide.GetValue_String(TEXT("PersonalRule"), szPersonalRule, std::size(szPersonalRule));

      // 扩展配置
      if (szPersonalRule[0] != 0) {
        INT nPersonalRuleSize = StrLenT(szPersonalRule) / 2;

        // 转换字符
        for (INT i = 0; i < nPersonalRuleSize; i++) {
          // 获取字符
          TCHAR cbChar1 = szPersonalRule[i * 2];
          TCHAR cbChar2 = szPersonalRule[i * 2 + 1];

          // 效验字符
          ASSERT((cbChar1 >= TEXT('0')) && (cbChar1 <= TEXT('9')) || (cbChar1 >= TEXT('A')) && (cbChar1 <= TEXT('F')));
          ASSERT((cbChar2 >= TEXT('0')) && (cbChar2 <= TEXT('9')) || (cbChar2 >= TEXT('A')) && (cbChar2 <= TEXT('F')));

          // 生成结果
          if ((cbChar2 >= TEXT('0')) && (cbChar2 <= TEXT('9')))
            GetPersonalRule.cbPersonalRule[i] += (cbChar2 - TEXT('0'));
          if ((cbChar2 >= TEXT('A')) && (cbChar2 <= TEXT('F')))
            GetPersonalRule.cbPersonalRule[i] += (cbChar2 - TEXT('A') + 0x0A);

          // 生成结果
          if ((cbChar1 >= TEXT('0')) && (cbChar1 <= TEXT('9')))
            GetPersonalRule.cbPersonalRule[i] += (cbChar1 - TEXT('0')) * 0x10;
          if ((cbChar1 >= TEXT('A')) && (cbChar1 <= TEXT('F')))
            GetPersonalRule.cbPersonalRule[i] += (cbChar1 - TEXT('A') + 0x0A) * 0x10;
        }

        EmitDataBaseEngineResult(DBO_MB_GET_PERSONAL_RULE, dwContextID, &GetPersonalRule, sizeof(tagGetPersonalRule));
      }
    } else {
      // 错误信息
      CLogger::Error(TEXT("获取私人房定制配置出错！"));

      // 操作结果
      OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("数据库异常,请稍后再试！"), false, false);
    }
    return true;

  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 操作结果
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("数据库异常,请稍后再试！"), false, false);
    return false;
  }

  return true;
}

// 写入房间结束时间
bool CDataBaseEngineSink::OnRequestCloseRoomWriteDissumeTime(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    DBR_GR_CLOSE_ROOM_SERVER_ID* pCloseRoomID = (DBR_GR_CLOSE_ROOM_SERVER_ID*)pData;
    ASSERT(wDataSize == sizeof(DBR_GR_CLOSE_ROOM_SERVER_ID));
    if (wDataSize != sizeof(DBR_GR_CLOSE_ROOM_SERVER_ID))
      return false;
    // 构造参数
    m_PlatformDBAide.ResetParameter();

    // 用户信息
    m_PlatformDBAide.AddParameter(TEXT("@dwServerID"), pCloseRoomID->dwServerID);

    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GS_CloseRoomWriteDissumeTime"), true);
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

// 私人配置
bool CDataBaseEngineSink::OnRequestQueryUserRoomScore(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    DBR_GR_QUERY_USER_ROOM_INFO* pQueryUserRoomInfo = (DBR_GR_QUERY_USER_ROOM_INFO*)pData;

    // 构造参数
    m_PlatformDBAide.ResetParameter();

    // 用户信息
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pQueryUserRoomInfo->dwUserID);

    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GS_GetPersonalRoomUserScore"), true);

    if (lResultCode == DB_SUCCESS) {
      // 变量定义
      WORD dwPaketSize = 0;
      BYTE cbBuffer[MAX_ASYNCHRONISM_DATA];
      tagQueryPersonalRoomUserScore* pPersonalRoomInfo = nullptr;
      ZeroMemory(cbBuffer, sizeof(cbBuffer));
      int nCount = 0;
      while (!m_PlatformDBModule->IsRecordsetEnd()) {
        if (nCount >= MAX_CREATE_PERSONAL_ROOM || (dwPaketSize + sizeof(tagQueryPersonalRoomUserScore)) >= 16385) {
          break;
        }
        // 将配置信息改为制定的配置信息
        DWORD dwPersonalRoomID = 0;
        dwPersonalRoomID = m_PlatformDBAide.GetValue_DWORD(TEXT("RoomID"));

        if (dwPersonalRoomID == 0) {
          // 移动记录
          m_PlatformDBModule->MoveToNext();
          continue;
        }

        // 读取对应的房间记录
        m_TreasureDBAide.ResetParameter();
        m_TreasureDBAide.AddParameter(TEXT("@dwPersonalRoomID"), dwPersonalRoomID);

        // 读取信息
        pPersonalRoomInfo = (tagQueryPersonalRoomUserScore*)(cbBuffer + dwPaketSize);

        // 输出参数
        TCHAR szDescribeString[128] = TEXT("");
        m_TreasureDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

        // 执行查询
        LONG lResultCode = m_TreasureDBAide.ExecuteProcess(TEXT("GSP_GS_QueryPersonalRoomInfo"), true);
        if (DB_SUCCESS == lResultCode) {
          pPersonalRoomInfo->dwPersonalRoomID = dwPersonalRoomID;
          m_TreasureDBAide.GetValue_String(TEXT("UserNicname"), pPersonalRoomInfo->szUserNicname, sizeof(pPersonalRoomInfo->szUserNicname));
          pPersonalRoomInfo->dwPlayTimeLimit = m_TreasureDBAide.GetValue_DWORD(TEXT("dwPlayTimeLimit"));
          pPersonalRoomInfo->dwPlayTurnCount = m_TreasureDBAide.GetValue_DWORD(TEXT("dwPlayTurnCount"));
          BYTE cbIsDisssumRoom = m_TreasureDBAide.GetValue_BYTE(TEXT("cbIsDisssumRoom"));
          m_TreasureDBAide.GetValue_SystemTime(TEXT("sysCreateTime"), pPersonalRoomInfo->sysCreateTime);
          m_TreasureDBAide.GetValue_SystemTime(TEXT("sysDissumeTime"), pPersonalRoomInfo->sysDissumeTime);

          // 如果房间未解散 将 结束时间 置0
          if (!cbIsDisssumRoom) {
            memset(&(pPersonalRoomInfo->sysDissumeTime), 0, sizeof(pPersonalRoomInfo->sysDissumeTime));
          }
          // if (cbIsDisssumRoom)
          {
            // 读取二进制数据并转化为相应数据结构
            BYTE szTempBinary[1858] = {0};
            TCHAR szTempChar[3712] = {0};
            m_TreasureDBAide.GetValue_String(TEXT("strRoomScoreInfo"), szTempChar, std::size(szTempChar));
            std::string strTempChar = ToSimpleUtf8(szTempChar);
            // 换成 std::string 之后第二个参数就不能用以前的了，得跟第一个参数走
            HexStr2CharStr(strTempChar.c_str(), strTempChar.length(), szTempBinary);

            //
            for (int j = 0; j < PERSONAL_ROOM_CHAIR; j++) {
              memcpy(&pPersonalRoomInfo->PersonalUserScoreInfo[j], szTempBinary + j * sizeof(tagPersonalUserScoreInfo),
                     sizeof(tagPersonalUserScoreInfo));
            }

            // 设置位移
            dwPaketSize += sizeof(tagQueryPersonalRoomUserScore);
            nCount++;
          }
        }

        // 移动记录
        m_PlatformDBModule->MoveToNext();
      }
      EmitDataBaseEngineResult(DBO_GR_QUERY_USER_ROOM_SCORE, dwContextID, cbBuffer, dwPaketSize);
    }

  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }

  return true;
}

// 帐号绑定
bool CDataBaseEngineSink::OnRequestAccountBind(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_AccountBind));
    if (wDataSize != sizeof(DBR_GP_AccountBind))
      return false;

    // 请求处理
    DBR_GP_AccountBind* pModifyMachine = (DBR_GP_AccountBind*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pModifyMachine->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pModifyMachine->dwUserID);
    m_AccountsDBAide.AddParameter(TEXT("@strPassword"), pModifyMachine->szPassword);
    m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_AccountsDBAide.AddParameter(TEXT("@strMachineID"), pModifyMachine->szMachineID);

    m_AccountsDBAide.AddParameter(TEXT("@strBindAccounts"), pModifyMachine->szBindNewAccounts);
    m_AccountsDBAide.AddParameter(TEXT("@strBindPassword"), pModifyMachine->szBindNewPassword);
    m_AccountsDBAide.AddParameter(TEXT("@strBindSpreader"), pModifyMachine->szBindNewSpreader);
    m_AccountsDBAide.AddParameter(TEXT("@cbDeviceType"), pModifyMachine->cbDeviceType);

    // 输出变量
    TCHAR szDescribe[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribe, sizeof(szDescribe), nanodbc::statement::PARAM_OUT);

    m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_AccountBind"), false);

    // 结果处理
    if (m_AccountsDBAide.GetReturnValue() == DB_SUCCESS) {
      // 变量定义
      DBO_GP_OperateSuccess OperateSuccess;
      ZeroMemory(&OperateSuccess, sizeof(OperateSuccess));

      // 获取信息
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

      // 构造变量
      OperateSuccess.lResultCode = m_AccountsDBModule->GetReturnValue();
      LSTRCPYN(OperateSuccess.szDescribeString, DBVarValue.c_str(), std::size(OperateSuccess.szDescribeString));

      // 发送结果
      WORD wStringSize = CountStringBuffer(OperateSuccess.szDescribeString);
      WORD wSendSize = sizeof(OperateSuccess) - sizeof(OperateSuccess.szDescribeString) + wStringSize;
      EmitDataBaseEngineResult(DBO_GP_OPERATE_SUCCESS, dwContextID, &OperateSuccess, wSendSize);
    } else {
      // 变量定义
      DBO_GP_OperateFailure OperateFailure;
      ZeroMemory(&OperateFailure, sizeof(OperateFailure));

      // 获取信息
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

      // 构造变量
      OperateFailure.lResultCode = m_AccountsDBModule->GetReturnValue();
      LSTRCPYN(OperateFailure.szDescribeString, DBVarValue.c_str(), std::size(OperateFailure.szDescribeString));

      // 发送结果
      WORD wStringSize = CountStringBuffer(OperateFailure.szDescribeString);
      WORD wSendSize = sizeof(OperateFailure) - sizeof(OperateFailure.szDescribeString) + wStringSize;
      EmitDataBaseEngineResult(DBO_GP_OPERATE_FAILURE, dwContextID, &OperateFailure, wSendSize);
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 结果处理
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试！"), false);

    return false;
  }

  return true;
}

// 帐号绑定
bool CDataBaseEngineSink::OnRequestAccountBindExists(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_AccountBind_Exists));
    if (wDataSize != sizeof(DBR_GP_AccountBind_Exists))
      return false;

    // 请求处理
    DBR_GP_AccountBind_Exists* pModifyMachine = (DBR_GP_AccountBind_Exists*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pModifyMachine->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pModifyMachine->dwUserID);
    m_AccountsDBAide.AddParameter(TEXT("@strPassword"), pModifyMachine->szPassword);
    m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_AccountsDBAide.AddParameter(TEXT("@strMachineID"), pModifyMachine->szMachineID);

    m_AccountsDBAide.AddParameter(TEXT("@strBindAccounts"), pModifyMachine->szBindExistsAccounts);
    m_AccountsDBAide.AddParameter(TEXT("@strBindPassword"), pModifyMachine->szBindExistsPassword);

    // 输出变量
    TCHAR szDescribe[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribe, sizeof(szDescribe), nanodbc::statement::PARAM_OUT);

    m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_AccountBindExists"), false);

    // 结果处理
    if (m_AccountsDBAide.GetReturnValue() == DB_SUCCESS) {
      // 变量定义
      DBO_GP_OperateSuccess OperateSuccess;
      ZeroMemory(&OperateSuccess, sizeof(OperateSuccess));

      // 获取信息
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

      // 构造变量
      OperateSuccess.lResultCode = m_AccountsDBModule->GetReturnValue();
      LSTRCPYN(OperateSuccess.szDescribeString, DBVarValue.c_str(), std::size(OperateSuccess.szDescribeString));

      // 发送结果
      WORD wStringSize = CountStringBuffer(OperateSuccess.szDescribeString);
      WORD wSendSize = sizeof(OperateSuccess) - sizeof(OperateSuccess.szDescribeString) + wStringSize;
      EmitDataBaseEngineResult(DBO_GP_OPERATE_SUCCESS, dwContextID, &OperateSuccess, wSendSize);
    } else {
      // 变量定义
      DBO_GP_OperateFailure OperateFailure;
      ZeroMemory(&OperateFailure, sizeof(OperateFailure));

      // 获取信息
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

      // 构造变量
      OperateFailure.lResultCode = m_AccountsDBModule->GetReturnValue();
      LSTRCPYN(OperateFailure.szDescribeString, DBVarValue.c_str(), std::size(OperateFailure.szDescribeString));

      // 发送结果
      WORD wStringSize = CountStringBuffer(OperateFailure.szDescribeString);
      WORD wSendSize = sizeof(OperateFailure) - sizeof(OperateFailure.szDescribeString) + wStringSize;
      EmitDataBaseEngineResult(DBO_GP_OPERATE_FAILURE, dwContextID, &OperateFailure, wSendSize);
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 结果处理
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试！"), false);

    return false;
  }

  return true;
}

// 获取参数
bool CDataBaseEngineSink::OnRequestGetParameter(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_GetParameter));
    if (wDataSize != sizeof(DBR_GP_GetParameter))
      return false;

    // 请求处理
    DBR_GP_GetParameter* pGetParameter = (DBR_GP_GetParameter*)pData;

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@wServerID"), pGetParameter->wServerID);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_AndroidGetParameter"), true);

    // 执行结果
    OnAndroidDisposeResult(dwContextID, lResultCode, SUB_GP_GET_PARAMETER, pGetParameter->wServerID);

    return true;
  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 操作结果
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("数据库异常,请稍后再试！"), false, false);

    return false;
  }

  return true;
}

// 添加参数
bool CDataBaseEngineSink::OnRequestAddParameter(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_AddParameter));
    if (wDataSize != sizeof(DBR_GP_AddParameter))
      return false;

    // 请求处理
    DBR_GP_AddParameter* pAddParameter = (DBR_GP_AddParameter*)pData;

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@wServerID"), pAddParameter->wServerID);
    m_AccountsDBAide.AddParameter(TEXT("@dwServiceMode"), pAddParameter->AndroidParameter.dwServiceMode);
    m_AccountsDBAide.AddParameter(TEXT("@dwAndroidCount"), pAddParameter->AndroidParameter.dwAndroidCount);
    m_AccountsDBAide.AddParameter(TEXT("@dwEnterTime"), pAddParameter->AndroidParameter.dwEnterTime);
    m_AccountsDBAide.AddParameter(TEXT("@dwLeaveTime"), pAddParameter->AndroidParameter.dwLeaveTime);
    m_AccountsDBAide.AddParameter(TEXT("@dwEnterMinInterval"), pAddParameter->AndroidParameter.dwEnterMinInterval);
    m_AccountsDBAide.AddParameter(TEXT("@dwEnterMaxInterval"), pAddParameter->AndroidParameter.dwEnterMaxInterval);
    m_AccountsDBAide.AddParameter(TEXT("@dwLeaveMinInterval"), pAddParameter->AndroidParameter.dwLeaveMinInterval);
    m_AccountsDBAide.AddParameter(TEXT("@dwLeaveMaxInterval"), pAddParameter->AndroidParameter.dwLeaveMaxInterval);
    m_AccountsDBAide.AddParameter(TEXT("@lTakeMinScore"), pAddParameter->AndroidParameter.lTakeMinScore);
    m_AccountsDBAide.AddParameter(TEXT("@lTakeMaxScore"), pAddParameter->AndroidParameter.lTakeMaxScore);
    m_AccountsDBAide.AddParameter(TEXT("@dwSwitchMinInnings"), pAddParameter->AndroidParameter.dwSwitchMinInnings);
    m_AccountsDBAide.AddParameter(TEXT("@dwSwitchMaxInnings"), pAddParameter->AndroidParameter.dwSwitchMaxInnings);
    m_AccountsDBAide.AddParameter(TEXT("@dwAndroidCountMember0"), pAddParameter->AndroidParameter.AndroidCountMember0);
    m_AccountsDBAide.AddParameter(TEXT("@dwAndroidCountMember1"), pAddParameter->AndroidParameter.AndroidCountMember1);
    m_AccountsDBAide.AddParameter(TEXT("@dwAndroidCountMember2"), pAddParameter->AndroidParameter.AndroidCountMember2);
    m_AccountsDBAide.AddParameter(TEXT("@dwAndroidCountMember3"), pAddParameter->AndroidParameter.AndroidCountMember3);
    m_AccountsDBAide.AddParameter(TEXT("@dwAndroidCountMember4"), pAddParameter->AndroidParameter.AndroidCountMember4);
    m_AccountsDBAide.AddParameter(TEXT("@dwAndroidCountMember5"), pAddParameter->AndroidParameter.AndroidCountMember5);
    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_AndroidAddParameter"), true);

    // 执行结果
    OnAndroidDisposeResult(dwContextID, lResultCode, SUB_GP_ADD_PARAMETER, pAddParameter->wServerID);

    return true;
  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 操作结果
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("数据库异常,请稍后再试！"), false, false);

    return false;
  }

  return true;
}

// 修改参数
bool CDataBaseEngineSink::OnRequestModifyParameter(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_ModifyParameter));
    if (wDataSize != sizeof(DBR_GP_ModifyParameter))
      return false;

    // 请求处理
    DBR_GP_ModifyParameter* pModifyParameter = (DBR_GP_ModifyParameter*)pData;

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwDatchID"), pModifyParameter->AndroidParameter.dwBatchID);
    m_AccountsDBAide.AddParameter(TEXT("@dwServiceMode"), pModifyParameter->AndroidParameter.dwServiceMode);
    m_AccountsDBAide.AddParameter(TEXT("@dwAndroidCount"), pModifyParameter->AndroidParameter.dwAndroidCount);
    m_AccountsDBAide.AddParameter(TEXT("@dwEnterTime"), pModifyParameter->AndroidParameter.dwEnterTime);
    m_AccountsDBAide.AddParameter(TEXT("@dwLeaveTime"), pModifyParameter->AndroidParameter.dwLeaveTime);
    m_AccountsDBAide.AddParameter(TEXT("@dwEnterMinInterval"), pModifyParameter->AndroidParameter.dwEnterMinInterval);
    m_AccountsDBAide.AddParameter(TEXT("@dwEnterMaxInterval"), pModifyParameter->AndroidParameter.dwEnterMaxInterval);
    m_AccountsDBAide.AddParameter(TEXT("@dwLeaveMinInterval"), pModifyParameter->AndroidParameter.dwLeaveMinInterval);
    m_AccountsDBAide.AddParameter(TEXT("@dwLeaveMaxInterval"), pModifyParameter->AndroidParameter.dwLeaveMaxInterval);
    m_AccountsDBAide.AddParameter(TEXT("@lTakeMinScore"), pModifyParameter->AndroidParameter.lTakeMinScore);
    m_AccountsDBAide.AddParameter(TEXT("@lTakeMaxScore"), pModifyParameter->AndroidParameter.lTakeMaxScore);
    m_AccountsDBAide.AddParameter(TEXT("@dwSwitchMinInnings"), pModifyParameter->AndroidParameter.dwSwitchMinInnings);
    m_AccountsDBAide.AddParameter(TEXT("@dwSwitchMaxInnings"), pModifyParameter->AndroidParameter.dwSwitchMaxInnings);
    m_AccountsDBAide.AddParameter(TEXT("@dwAndroidCountMember0"), pModifyParameter->AndroidParameter.AndroidCountMember0);
    m_AccountsDBAide.AddParameter(TEXT("@dwAndroidCountMember1"), pModifyParameter->AndroidParameter.AndroidCountMember1);
    m_AccountsDBAide.AddParameter(TEXT("@dwAndroidCountMember2"), pModifyParameter->AndroidParameter.AndroidCountMember2);
    m_AccountsDBAide.AddParameter(TEXT("@dwAndroidCountMember3"), pModifyParameter->AndroidParameter.AndroidCountMember3);
    m_AccountsDBAide.AddParameter(TEXT("@dwAndroidCountMember4"), pModifyParameter->AndroidParameter.AndroidCountMember4);
    m_AccountsDBAide.AddParameter(TEXT("@dwAndroidCountMember5"), pModifyParameter->AndroidParameter.AndroidCountMember5);
    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_AndroidModifyParameter"), true);

    // 执行结果
    OnAndroidDisposeResult(dwContextID, lResultCode, SUB_GP_MODIFY_PARAMETER, pModifyParameter->wServerID);

    return true;
  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 操作结果
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("数据库异常,请稍后再试！"), false, false);

    return false;
  }

  return true;
}

// 删除参数
bool CDataBaseEngineSink::OnRequestDeleteParameter(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_DeleteParameter));
    if (wDataSize != sizeof(DBR_GP_DeleteParameter))
      return false;

    // 请求处理
    DBR_GP_DeleteParameter* pDeleteParameter = (DBR_GP_DeleteParameter*)pData;

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwBatchID"), pDeleteParameter->dwBatchID);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_AndroidDeleteParameter"), true);

    // 执行结果
    OnAndroidDisposeResult(dwContextID, lResultCode, SUB_GP_DELETE_PARAMETER, pDeleteParameter->wServerID);

    return true;
  } catch (IDataBaseException* pIException) {
    // 错误信息
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 操作结果
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("数据库异常,请稍后再试！"), false, false);

    return false;
  }

  return true;
}
// 绑定推广
bool CDataBaseEngineSink::OnRequestBindSpreader(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_BindSpreader));
    if (wDataSize != sizeof(DBR_GP_BindSpreader))
      return false;
    // 请求处理
    DBR_GP_BindSpreader* pBindSpreader = (DBR_GP_BindSpreader*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pBindSpreader->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pBindSpreader->dwUserID);
    m_AccountsDBAide.AddParameter(TEXT("@dwSpreaderID"), pBindSpreader->dwSpreaderID);
    m_AccountsDBAide.AddParameter(TEXT("@strPassword"), pBindSpreader->szPassword);
    m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 结果处理
    if (m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_BindSpreader"), true) == DB_SUCCESS) {
      DBO_GP_BindSpreaderResult BindSpreaderResult;
      ZeroMemory(&BindSpreaderResult, sizeof(BindSpreaderResult));
      // 获取参数
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

      BindSpreaderResult.dwDiamond = m_AccountsDBAide.GetValue_DWORD(TEXT("dwDiamond"));
      BindSpreaderResult.dwRewardDiamond = m_AccountsDBAide.GetValue_DWORD(TEXT("dwRewardDiamond"));
      LSTRCPYN(BindSpreaderResult.szDescribeString, DBVarValue.c_str(), std::size(BindSpreaderResult.szDescribeString));

      // 发送结果
      EmitDataBaseEngineResult(DBO_GP_BIND_SPREADER_RESULT, dwContextID, &BindSpreaderResult, sizeof(BindSpreaderResult));

    } else {
      // 获取参数
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

      // 错误处理
      OnOperateDisposeResult(dwContextID, m_AccountsDBAide.GetReturnValue(), DBVarValue.c_str(), false);
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 结果处理
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试！"), false);

    return false;
  }

  return true;
}
// 查询资料
bool CDataBaseEngineSink::OnRequestQueryIndividual(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_QueryIndividual));
    if (wDataSize != sizeof(DBR_GP_QueryIndividual))
      return false;

    // 请求处理
    DBR_GP_QueryIndividual* pQueryIndividual = (DBR_GP_QueryIndividual*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pQueryIndividual->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pQueryIndividual->dwUserID);
    m_AccountsDBAide.AddParameter(TEXT("@strPassword"), pQueryIndividual->szPassword);
    m_AccountsDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 结果处理
    if (m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_QueryUserIndividual"), true) == DB_SUCCESS) {
      // 变量定义
      DBO_GP_UserIndividual UserIndividual;
      ZeroMemory(&UserIndividual, sizeof(UserIndividual));

      // 用户信息
      UserIndividual.dwUserID = m_AccountsDBAide.GetValue_DWORD(TEXT("UserID"));
      m_AccountsDBAide.GetValue_String(TEXT("UserNote"), UserIndividual.szUserNote, std::size(UserIndividual.szUserNote));
      m_AccountsDBAide.GetValue_String(TEXT("Compellation"), UserIndividual.szCompellation, std::size(UserIndividual.szCompellation));
      m_AccountsDBAide.GetValue_String(TEXT("PassPortID"), UserIndividual.szPassPortID, std::size(UserIndividual.szPassPortID));

      // 电话号码
      m_AccountsDBAide.GetValue_String(TEXT("SeatPhone"), UserIndividual.szSeatPhone, std::size(UserIndividual.szSeatPhone));
      m_AccountsDBAide.GetValue_String(TEXT("MobilePhone"), UserIndividual.szMobilePhone, std::size(UserIndividual.szMobilePhone));

      // 联系资料
      m_AccountsDBAide.GetValue_String(TEXT("QQ"), UserIndividual.szQQ, std::size(UserIndividual.szQQ));
      m_AccountsDBAide.GetValue_String(TEXT("EMail"), UserIndividual.szEMail, std::size(UserIndividual.szEMail));
      m_AccountsDBAide.GetValue_String(TEXT("DwellingPlace"), UserIndividual.szDwellingPlace, std::size(UserIndividual.szDwellingPlace));

      // 推广信息
      m_AccountsDBAide.GetValue_String(TEXT("Spreader"), UserIndividual.szSpreader, std::size(UserIndividual.szSpreader));

      // 发送结果
      EmitDataBaseEngineResult(DBO_GP_USER_INDIVIDUAL, dwContextID, &UserIndividual, sizeof(UserIndividual));
    } else {
      // 获取参数
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

      // 错误处理
      OnOperateDisposeResult(dwContextID, m_AccountsDBAide.GetReturnValue(), DBVarValue.c_str(), false);
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 结果处理
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试！"), false);

    return false;
  }

  return true;
}

// 查询银行
bool CDataBaseEngineSink::OnRequestQueryInsureInfo(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_QueryInsureInfo));
    if (wDataSize != sizeof(DBR_GP_QueryInsureInfo))
      return false;

    // 请求处理
    DBR_GP_QueryInsureInfo* pQueryInsureInfo = (DBR_GP_QueryInsureInfo*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pQueryInsureInfo->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_TreasureDBAide.ResetParameter();
    m_TreasureDBAide.AddParameter(TEXT("@dwUserID"), pQueryInsureInfo->dwUserID);
    m_TreasureDBAide.AddParameter(TEXT("@strPassword"), pQueryInsureInfo->szPassword);
    m_TreasureDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_TreasureDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 结果处理
    if (m_TreasureDBAide.ExecuteProcess(TEXT("GSP_GR_QueryUserInsureInfo"), true) == DB_SUCCESS) {
      // 变量定义
      DBO_GP_UserInsureInfo UserInsureInfo;
      ZeroMemory(&UserInsureInfo, sizeof(UserInsureInfo));

      // 银行信息
      UserInsureInfo.cbEnjoinTransfer = m_TreasureDBAide.GetValue_BYTE(TEXT("EnjoinTransfer"));
      UserInsureInfo.wRevenueTake = m_TreasureDBAide.GetValue_WORD(TEXT("RevenueTake"));
      UserInsureInfo.wRevenueTransfer = m_TreasureDBAide.GetValue_WORD(TEXT("RevenueTransfer"));
      UserInsureInfo.wRevenueTransferMember = m_TreasureDBAide.GetValue_WORD(TEXT("RevenueTransferMember"));
      UserInsureInfo.wRevenueTransfer = m_TreasureDBAide.GetValue_WORD(TEXT("RevenueTransfer"));
      UserInsureInfo.wServerID = m_TreasureDBAide.GetValue_WORD(TEXT("ServerID"));
      UserInsureInfo.lUserScore = m_TreasureDBAide.GetValue_LONGLONG(TEXT("Score"));
      UserInsureInfo.lUserInsure = m_TreasureDBAide.GetValue_LONGLONG(TEXT("Insure"));
      UserInsureInfo.lTransferPrerequisite = m_TreasureDBAide.GetValue_LONGLONG(TEXT("TransferPrerequisite"));
      UserInsureInfo.dwUserRight = m_TreasureDBAide.GetValue_INT(TEXT("UserRight"));

      // 发送结果
      EmitDataBaseEngineResult(DBO_GP_USER_INSURE_INFO, dwContextID, &UserInsureInfo, sizeof(UserInsureInfo));
    } else {
      // 获取参数
      StringT DBVarValue;
      m_TreasureDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

      // 错误处理
      OnInsureDisposeResult(dwContextID, m_TreasureDBAide.GetReturnValue(), DBVarValue.c_str(), false);
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 结果处理
    OnOperateDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试！"), false);

    return false;
  }

  return true;
}

// 查询用户
bool CDataBaseEngineSink::OnRequestQueryTransferUserInfo(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize == sizeof(DBR_GP_QueryInsureUserInfo));
  if (wDataSize != sizeof(DBR_GP_QueryInsureUserInfo))
    return false;

  // 请求处理
  DBR_GP_QueryInsureUserInfo* pQueryTransferUserInfo = (DBR_GP_QueryInsureUserInfo*)pData;

  try {
    // 构造参数
    m_TreasureDBAide.ResetParameter();
    m_TreasureDBAide.AddParameter(TEXT("@cbByNickName"), pQueryTransferUserInfo->cbByNickName);
    m_TreasureDBAide.AddParameter(TEXT("@strAccounts"), pQueryTransferUserInfo->szAccounts);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_TreasureDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 结果处理
    if (m_TreasureDBAide.ExecuteProcess(TEXT("GSP_GR_QueryTransferUserInfo"), true) == DB_SUCCESS) {
      // 变量定义
      DBO_GP_UserTransferUserInfo TransferUserInfo;
      ZeroMemory(&TransferUserInfo, sizeof(TransferUserInfo));

      // 银行信息
      TransferUserInfo.dwGameID = m_TreasureDBAide.GetValue_DWORD(TEXT("GameID"));
      m_TreasureDBAide.GetValue_String(TEXT("Accounts"), TransferUserInfo.szAccounts, std::size(TransferUserInfo.szAccounts));

      // 发送结果
      EmitDataBaseEngineResult(DBO_GP_USER_INSURE_USER_INFO, dwContextID, &TransferUserInfo, sizeof(TransferUserInfo));
    } else {
      // 获取参数
      StringT DBVarValue;
      m_TreasureDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

      // 错误处理
      OnInsureDisposeResult(dwContextID, m_TreasureDBAide.GetReturnValue(), DBVarValue.c_str(), false);
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 结果处理
    OnInsureDisposeResult(dwContextID, DB_ERROR, TEXT("由于数据库操作异常，请您稍后重试！"), false);

    return false;
  }
}

bool CDataBaseEngineSink::OnRequestQueryTransferRebate(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize == sizeof(DBR_GP_QueryTransferRebate));
  if (wDataSize != sizeof(DBR_GP_QueryTransferRebate))
    return false;

  // 请求处理
  DBR_GP_QueryTransferRebate* pTransferRebate = (DBR_GP_QueryTransferRebate*)pData;

  try {
    // 构造参数
    m_TreasureDBAide.ResetParameter();
    m_TreasureDBAide.AddParameter(TEXT("@dwUserID"), pTransferRebate->dwUserID);
    m_TreasureDBAide.AddParameter(TEXT("@strPassword"), pTransferRebate->szPassword);

    // 变量定义
    DBO_GP_QueryTransferRebateResult RebateResult;
    ZeroMemory(&RebateResult, sizeof(RebateResult));
    // 结果处理
    if (m_TreasureDBAide.ExecuteProcess(TEXT("GSP_GR_QueryTransferRebate"), true) == DB_SUCCESS) {
      // 银行信息
      RebateResult.dwUserID = m_TreasureDBAide.GetValue_DWORD(TEXT("UserID"));
      // RebateResult.cbRebateEnabled=m_TreasureDBAide.GetValue_DWORD(TEXT("RebateEnabled"));
      // RebateResult.lIngot=m_TreasureDBAide.GetValue_LONGLONG(TEXT("Ingot"));
      // RebateResult.lLoveLiness=m_TreasureDBAide.GetValue_LONGLONG(TEXT("LoveLiness"));
    }
    // 发送结果
    EmitDataBaseEngineResult(DBO_GP_QUERY_TRANSFER_REBATE_RESULT, dwContextID, &RebateResult, sizeof(RebateResult));
    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }

  return true;
}

// 加载奖励
bool CDataBaseEngineSink::OnRequestCheckInReward(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 构造参数
    m_PlatformDBAide.ResetParameter();

    // 执行命令
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_LoadCheckInReward"), true);

    // 构造结构
    DBO_GP_CheckInReward CheckInReward;
    ZeroMemory(&CheckInReward, sizeof(CheckInReward));

    // 执行成功
    if (lResultCode == DB_SUCCESS) {
      // 变量定义
      WORD wDayIndex = 0;
      while (!m_PlatformDBModule->IsRecordsetEnd()) {
        wDayIndex = m_PlatformDBAide.GetValue_WORD(TEXT("DayID"));
        if (wDayIndex <= LEN_WEEK) {
          CheckInReward.lRewardGold[wDayIndex - 1] = m_PlatformDBAide.GetValue_LONGLONG(TEXT("RewardGold"));
        }

        // 移动记录
        m_PlatformDBModule->MoveToNext();
      }

      // 投递结果
      EmitDataBaseEngineResult(DBO_GP_CHECKIN_REWARD, dwContextID, &CheckInReward, sizeof(CheckInReward));
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }

  return true;
}

// 查询签到
bool CDataBaseEngineSink::OnRequestCheckInQueryInfo(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_CheckInQueryInfo));
    if (wDataSize != sizeof(DBR_GP_CheckInQueryInfo))
      return false;

    // 请求处理
    DBR_GP_CheckInQueryInfo* pCheckInQueryInfo = (DBR_GP_CheckInQueryInfo*)pData;

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pCheckInQueryInfo->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@strPassword"), pCheckInQueryInfo->szPassword);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 结果处理
    if (m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GR_CheckInQueryInfo"), true) == DB_SUCCESS) {
      // 变量定义
      DBO_GP_CheckInInfo CheckInInfo;
      ZeroMemory(&CheckInInfo, sizeof(CheckInInfo));

      // 银行信息
      CheckInInfo.wSeriesDate = m_PlatformDBAide.GetValue_WORD(TEXT("SeriesDate"));
      CheckInInfo.bTodayChecked = (m_PlatformDBAide.GetValue_BYTE(TEXT("TodayCheckIned")) == TRUE);

      // 发送结果
      EmitDataBaseEngineResult(DBO_GP_CHECKIN_INFO, dwContextID, &CheckInInfo, sizeof(CheckInInfo));
    } else {
      // 获取参数
      StringT DBVarValue;
      m_PlatformDBAide.GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

      // 构造结构
      DBO_GP_CheckInResult CheckInResult;
      CheckInResult.bSuccessed = false;
      LSTRCPYN(CheckInResult.szNotifyContent, DBVarValue.c_str(), std::size(CheckInResult.szNotifyContent));

      // 发送结果
      WORD wSendSize = sizeof(CheckInResult) - sizeof(CheckInResult.szNotifyContent) + CountStringBuffer(CheckInResult.szNotifyContent);
      EmitDataBaseEngineResult(DBO_GP_CHECKIN_RESULT, dwContextID, &CheckInResult, wSendSize);
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 构造结构
    DBO_GP_CheckInResult CheckInResult;
    CheckInResult.bSuccessed = false;
    LSTRCPYN(CheckInResult.szNotifyContent, TEXT("由于数据库操作异常，请您稍后重试！"), std::size(CheckInResult.szNotifyContent));

    // 发送结果
    WORD wSendSize = sizeof(CheckInResult) - sizeof(CheckInResult.szNotifyContent) + CountStringBuffer(CheckInResult.szNotifyContent);
    EmitDataBaseEngineResult(DBO_GP_CHECKIN_RESULT, dwContextID, &CheckInResult, wSendSize);

    return false;
  }
}

// 执行签到
bool CDataBaseEngineSink::OnRequestCheckInDone(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_CheckInDone));
    if (wDataSize != sizeof(DBR_GP_CheckInDone))
      return false;

    // 请求处理
    DBR_GP_CheckInDone* pCheckInDone = (DBR_GP_CheckInDone*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pCheckInDone->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pCheckInDone->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@strPassword"), pCheckInDone->szPassword);
    m_PlatformDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_PlatformDBAide.AddParameter(TEXT("@strMachineID"), pCheckInDone->szMachineID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行脚本
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GR_CheckInDone"), true);

    // 变量定义
    DBO_GP_CheckInResult CheckInResult;
    ZeroMemory(&CheckInResult, sizeof(CheckInResult));

    // 读取分数
    if (lResultCode == DB_SUCCESS && !m_PlatformDBModule->IsRecordsetEnd()) {
      CheckInResult.lScore = m_PlatformDBAide.GetValue_LONGLONG(TEXT("Score"));
    }

    // 获取参数
    StringT DBVarValue;
    m_PlatformDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

    // 银行信息
    CheckInResult.bSuccessed = lResultCode == DB_SUCCESS;
    LSTRCPYN(CheckInResult.szNotifyContent, DBVarValue.c_str(), std::size(CheckInResult.szNotifyContent));

    // 发送结果
    WORD wSendSize = sizeof(CheckInResult) - sizeof(CheckInResult.szNotifyContent) + CountStringBuffer(CheckInResult.szNotifyContent);
    EmitDataBaseEngineResult(DBO_GP_CHECKIN_RESULT, dwContextID, &CheckInResult, wSendSize);

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 构造结构
    DBO_GP_CheckInResult CheckInResult;
    CheckInResult.bSuccessed = false;
    LSTRCPYN(CheckInResult.szNotifyContent, TEXT("由于数据库操作异常，请您稍后重试！"), std::size(CheckInResult.szNotifyContent));

    // 发送结果
    WORD wSendSize = sizeof(CheckInResult) - sizeof(CheckInResult.szNotifyContent) + CountStringBuffer(CheckInResult.szNotifyContent);
    EmitDataBaseEngineResult(DBO_GP_CHECKIN_RESULT, dwContextID, &CheckInResult, wSendSize);

    return false;
  }
}

// 放弃任务
bool CDataBaseEngineSink::OnRequestTaskGiveUp(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_TaskGiveUP));
    if (wDataSize != sizeof(DBR_GP_TaskGiveUP))
      return false;

    // 请求处理
    DBR_GP_TaskGiveUP* pTake = (DBR_GP_TaskGiveUP*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pTake->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pTake->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@wTaskID"), pTake->wTaskID);
    m_PlatformDBAide.AddParameter(TEXT("@strPassword"), pTake->szPassword);
    m_PlatformDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_PlatformDBAide.AddParameter(TEXT("@strMachineID"), pTake->szMachineID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行脚本
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GR_TaskGiveUp"), false);

    // 变量定义
    DBO_GP_TaskResult TaskResult;
    ZeroMemory(&TaskResult, sizeof(TaskResult));

    // 获取参数
    StringT DBVarValue;
    m_PlatformDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

    // 银行信息
    TaskResult.wCommandID = SUB_GP_TASK_GIVEUP;
    TaskResult.bSuccessed = lResultCode == DB_SUCCESS;
    LSTRCPYN(TaskResult.szNotifyContent, DBVarValue.c_str(), std::size(TaskResult.szNotifyContent));

    // 发送结果
    WORD wSendSize = sizeof(TaskResult) - sizeof(TaskResult.szNotifyContent) + CountStringBuffer(TaskResult.szNotifyContent);
    EmitDataBaseEngineResult(DBO_GP_TASK_RESULT, dwContextID, &TaskResult, wSendSize);

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 构造结构
    DBO_GP_TaskResult TaskResult;
    TaskResult.bSuccessed = false;
    TaskResult.wCommandID = SUB_GP_TASK_TAKE;
    LSTRCPYN(TaskResult.szNotifyContent, TEXT("由于数据库操作异常，请您稍后重试！"), std::size(TaskResult.szNotifyContent));

    // 发送结果
    WORD wSendSize = sizeof(TaskResult) - sizeof(TaskResult.szNotifyContent) + CountStringBuffer(TaskResult.szNotifyContent);
    EmitDataBaseEngineResult(DBO_GP_TASK_RESULT, dwContextID, &TaskResult, wSendSize);

    return false;
  }
}

// 领取任务
bool CDataBaseEngineSink::OnRequestTaskTake(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_TaskTake));
    if (wDataSize != sizeof(DBR_GP_TaskTake))
      return false;

    // 请求处理
    DBR_GP_TaskTake* pTaskTake = (DBR_GP_TaskTake*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pTaskTake->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pTaskTake->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@wTaskID"), pTaskTake->wTaskID);
    m_PlatformDBAide.AddParameter(TEXT("@strPassword"), pTaskTake->szPassword);
    m_PlatformDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_PlatformDBAide.AddParameter(TEXT("@strMachineID"), pTaskTake->szMachineID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行脚本
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GR_TaskTake"), false);

    // 变量定义
    DBO_GP_TaskResult TaskResult;
    ZeroMemory(&TaskResult, sizeof(TaskResult));

    // 获取参数
    StringT DBVarValue;
    m_PlatformDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

    // 银行信息
    TaskResult.wCommandID = SUB_GP_TASK_TAKE;
    TaskResult.bSuccessed = lResultCode == DB_SUCCESS;
    LSTRCPYN(TaskResult.szNotifyContent, DBVarValue.c_str(), std::size(TaskResult.szNotifyContent));

    // 发送结果
    WORD wSendSize = sizeof(TaskResult) - sizeof(TaskResult.szNotifyContent) + CountStringBuffer(TaskResult.szNotifyContent);
    EmitDataBaseEngineResult(DBO_GP_TASK_RESULT, dwContextID, &TaskResult, wSendSize);

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 构造结构
    DBO_GP_TaskResult TaskResult;
    TaskResult.bSuccessed = false;
    TaskResult.wCommandID = SUB_GP_TASK_TAKE;
    LSTRCPYN(TaskResult.szNotifyContent, TEXT("由于数据库操作异常，请您稍后重试！"), std::size(TaskResult.szNotifyContent));

    // 发送结果
    WORD wSendSize = sizeof(TaskResult) - sizeof(TaskResult.szNotifyContent) + CountStringBuffer(TaskResult.szNotifyContent);
    EmitDataBaseEngineResult(DBO_GP_TASK_RESULT, dwContextID, &TaskResult, wSendSize);

    return false;
  }
}

// 领取奖励
bool CDataBaseEngineSink::OnRequestTaskReward(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_TaskReward));
    if (wDataSize != sizeof(DBR_GP_TaskReward))
      return false;

    // 请求处理
    DBR_GP_TaskReward* pTaskReward = (DBR_GP_TaskReward*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pTaskReward->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pTaskReward->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@wTaskID"), pTaskReward->wTaskID);
    m_PlatformDBAide.AddParameter(TEXT("@strPassword"), pTaskReward->szPassword);
    m_PlatformDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_PlatformDBAide.AddParameter(TEXT("@strMachineID"), pTaskReward->szMachineID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行脚本
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GR_TaskReward"), true);

    // 变量定义
    DBO_GP_TaskResult TaskResult;
    ZeroMemory(&TaskResult, sizeof(TaskResult));

    // 获取参数
    StringT DBVarValue;
    m_PlatformDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

    // 银行信息
    TaskResult.wCommandID = SUB_GP_TASK_REWARD;
    TaskResult.bSuccessed = lResultCode == DB_SUCCESS;
    LSTRCPYN(TaskResult.szNotifyContent, DBVarValue.c_str(), std::size(TaskResult.szNotifyContent));

    // 获取分数
    if (TaskResult.bSuccessed == true) {
      TaskResult.lCurrScore = m_PlatformDBAide.GetValue_LONGLONG(TEXT("Score"));
      TaskResult.lCurrIngot = m_PlatformDBAide.GetValue_LONGLONG(TEXT("Ingot"));
    }

    // 发送结果
    WORD wSendSize = sizeof(TaskResult) - sizeof(TaskResult.szNotifyContent) + CountStringBuffer(TaskResult.szNotifyContent);
    EmitDataBaseEngineResult(DBO_GP_TASK_RESULT, dwContextID, &TaskResult, wSendSize);

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 构造结构
    DBO_GP_TaskResult TaskResult;
    TaskResult.bSuccessed = false;
    TaskResult.wCommandID = SUB_GP_TASK_REWARD;
    LSTRCPYN(TaskResult.szNotifyContent, TEXT("由于数据库操作异常，请您稍后重试！"), std::size(TaskResult.szNotifyContent));

    // 发送结果
    WORD wSendSize = sizeof(TaskResult) - sizeof(TaskResult.szNotifyContent) + CountStringBuffer(TaskResult.szNotifyContent);
    EmitDataBaseEngineResult(DBO_GP_TASK_RESULT, dwContextID, &TaskResult, wSendSize);

    return false;
  }
}

// 查询任务
bool CDataBaseEngineSink::OnRequestTaskQueryInfo(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_TaskQueryInfo));
    if (wDataSize != sizeof(DBR_GP_TaskQueryInfo))
      return false;

    // 请求处理
    DBR_GP_TaskQueryInfo* pTaskQueryInfo = (DBR_GP_TaskQueryInfo*)pData;

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@wKindID"), 0);
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pTaskQueryInfo->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@strPassword"), pTaskQueryInfo->szPassword);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行脚本
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GR_QueryTaskInfo"), true);

    // 执行成功
    if (lResultCode == DB_SUCCESS) {
      // 变量定义
      DBO_GP_TaskInfo TaskInfo;
      tagTaskStatus* pTaskStatus = nullptr;
      ZeroMemory(&TaskInfo, sizeof(TaskInfo));

      // 变量定义
      while (m_PlatformDBModule->IsRecordsetEnd()) {
        // 设置变量
        pTaskStatus = &TaskInfo.TaskStatus[TaskInfo.wTaskCount++];

        // 读取数据
        pTaskStatus->wTaskID = m_PlatformDBAide.GetValue_WORD(TEXT("TaskID"));
        pTaskStatus->cbTaskStatus = m_PlatformDBAide.GetValue_BYTE(TEXT("TaskStatus"));
        pTaskStatus->wTaskProgress = m_PlatformDBAide.GetValue_WORD(TEXT("Progress"));

        // 移动记录
        m_PlatformDBModule->MoveToNext();
      }

      // 发送结果
      WORD wSendDataSize = sizeof(TaskInfo) - sizeof(TaskInfo.TaskStatus);
      wSendDataSize += sizeof(TaskInfo.TaskStatus[0]) * TaskInfo.wTaskCount;
      EmitDataBaseEngineResult(DBO_GP_TASK_INFO, dwContextID, &TaskInfo, wSendDataSize);
    } else {
      // 变量定义
      DBO_GP_TaskResult TaskResult;
      ZeroMemory(&TaskResult, sizeof(TaskResult));

      // 获取参数
      StringT DBVarValue;
      m_PlatformDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);

      // 银行信息
      TaskResult.bSuccessed = false;
      TaskResult.wCommandID = SUB_GP_TASK_LOAD;
      LSTRCPYN(TaskResult.szNotifyContent, DBVarValue.c_str(), std::size(TaskResult.szNotifyContent));

      // 发送结果
      WORD wSendSize = sizeof(TaskResult) - sizeof(TaskResult.szNotifyContent) + CountStringBuffer(TaskResult.szNotifyContent);
      EmitDataBaseEngineResult(DBO_GP_TASK_RESULT, dwContextID, &TaskResult, wSendSize);
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 构造结构
    DBO_GP_TaskResult TaskResult;
    TaskResult.bSuccessed = false;
    TaskResult.wCommandID = SUB_GP_TASK_LOAD;
    LSTRCPYN(TaskResult.szNotifyContent, TEXT("由于数据库操作异常，请您稍后重试！"), std::size(TaskResult.szNotifyContent));

    // 发送结果
    WORD wSendSize = sizeof(TaskResult) - sizeof(TaskResult.szNotifyContent) + CountStringBuffer(TaskResult.szNotifyContent);
    EmitDataBaseEngineResult(DBO_GP_TASK_RESULT, dwContextID, &TaskResult, wSendSize);

    return false;
  }
}

// 低保参数
bool CDataBaseEngineSink::OnRequestLoadBaseEnsure(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 构造参数
    m_PlatformDBAide.ResetParameter();

    // 执行命令
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_LoadBaseEnsure"), true);

    // 执行成功
    if (lResultCode == DB_SUCCESS) {
      // 构造结构
      DBO_GP_BaseEnsureParameter BaseEnsureParameter;
      ZeroMemory(&BaseEnsureParameter, sizeof(BaseEnsureParameter));

      // 变量定义
      if (!m_PlatformDBModule->IsRecordsetEnd()) {
        BaseEnsureParameter.cbTakeTimes = m_PlatformDBAide.GetValue_BYTE(TEXT("TakeTimes"));
        BaseEnsureParameter.lScoreAmount = m_PlatformDBAide.GetValue_WORD(TEXT("ScoreAmount"));
        BaseEnsureParameter.lScoreCondition = m_PlatformDBAide.GetValue_WORD(TEXT("ScoreCondition"));

        // 移动记录
        m_PlatformDBModule->MoveToNext();
      }

      // 投递结果
      EmitDataBaseEngineResult(DBO_GP_BASEENSURE_PARAMETER, dwContextID, &BaseEnsureParameter, sizeof(BaseEnsureParameter));
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
}

// 领取低保
bool CDataBaseEngineSink::OnRequestTakeBaseEnsure(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_TakeBaseEnsure));
    if (wDataSize != sizeof(DBR_GP_TakeBaseEnsure))
      return false;

    // 提取数据
    DBR_GP_TakeBaseEnsure* pTakeBaseEnsure = (DBR_GP_TakeBaseEnsure*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pTakeBaseEnsure->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pTakeBaseEnsure->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@strPassword"), pTakeBaseEnsure->szPassword);
    m_PlatformDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_PlatformDBAide.AddParameter(TEXT("@strMachineID"), pTakeBaseEnsure->szMachineID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strNotifyContent"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行脚本
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_TakeBaseEnsure"), true);

    // 变量定义
    DBO_GP_BaseEnsureResult BaseEnsureResult;
    ZeroMemory(&BaseEnsureResult, sizeof(BaseEnsureResult));

    // 读取分数
    if (lResultCode == DB_SUCCESS && !m_PlatformDBModule->IsRecordsetEnd()) {
      BaseEnsureResult.lGameScore = m_PlatformDBAide.GetValue_LONGLONG(TEXT("Score"));
    }

    // 获取参数
    StringT DBVarValue;
    m_PlatformDBModule->GetParameter(TEXT("@strNotifyContent"), DBVarValue);

    // 银行信息
    BaseEnsureResult.bSuccessed = lResultCode == DB_SUCCESS;
    LSTRCPYN(BaseEnsureResult.szNotifyContent, DBVarValue.c_str(), std::size(BaseEnsureResult.szNotifyContent));

    // 发送结果
    WORD wSendSize = sizeof(BaseEnsureResult) - sizeof(BaseEnsureResult.szNotifyContent);
    wSendSize += CountStringBuffer(BaseEnsureResult.szNotifyContent);
    EmitDataBaseEngineResult(DBO_GP_BASEENSURE_RESULT, dwContextID, &BaseEnsureResult, wSendSize);

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 构造结构
    DBO_GP_BaseEnsureResult BaseEnsureResult;
    ZeroMemory(&BaseEnsureResult, sizeof(BaseEnsureResult));

    // 设置变量
    BaseEnsureResult.bSuccessed = false;
    LSTRCPYN(BaseEnsureResult.szNotifyContent, TEXT("由于数据库操作异常，请您稍后重试！"), std::size(BaseEnsureResult.szNotifyContent));

    // 发送结果
    WORD wSendSize = sizeof(BaseEnsureResult) - sizeof(BaseEnsureResult.szNotifyContent);
    wSendSize += CountStringBuffer(BaseEnsureResult.szNotifyContent);
    EmitDataBaseEngineResult(DBO_GP_BASEENSURE_RESULT, dwContextID, &BaseEnsureResult, wSendSize);

    return false;
  }

  return true;
}

// 推广信息
bool CDataBaseEngineSink::OnRequestQuerySpreadInfo(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 提取数据
    DBR_GP_QuerySpreadInfo* pLoadSpreadInfo = (DBR_GP_QuerySpreadInfo*)pData;

    // 构造参数
    m_TreasureDBAide.ResetParameter();
    m_TreasureDBAide.AddParameter(TEXT("@dwUserID"), pLoadSpreadInfo->dwUserID);

    // 执行命令
    LONG lResultCode = m_TreasureDBAide.ExecuteProcess(TEXT("GSP_GR_LoadSpreadInfo"), true);

    // 执行成功
    if (lResultCode == DB_SUCCESS) {
      // 构造结构
      DBO_GP_UserSpreadInfo UserSpreadInfo;
      ZeroMemory(&UserSpreadInfo, sizeof(UserSpreadInfo));

      // 变量定义
      if (!m_TreasureDBModule->IsRecordsetEnd()) {
        UserSpreadInfo.dwSpreadCount = m_TreasureDBAide.GetValue_DWORD(TEXT("SpreadCount"));
        UserSpreadInfo.lSpreadReward = m_TreasureDBAide.GetValue_LONGLONG(TEXT("SpreadReward"));

        // 移动记录
        m_TreasureDBModule->MoveToNext();
      }

      // 投递结果
      EmitDataBaseEngineResult(DBO_GP_SPREAD_INFO, dwContextID, &UserSpreadInfo, sizeof(UserSpreadInfo));
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
}

bool CDataBaseEngineSink::OnRequestLoadRealAuth(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 构造参数
    m_PlatformDBAide.ResetParameter();

    // 执行命令
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_LoadRealAuth"), true);

    // 执行成功
    if (lResultCode == DB_SUCCESS) {
      // 构造结构
      DBO_GP_RealAuthParameter CmdParameter;
      ZeroMemory(&CmdParameter, sizeof(CmdParameter));
      CmdParameter.dwAuthRealAward = m_PlatformDBAide.GetValue_INT(TEXT("AuthRealAward"));
      CmdParameter.dwAuthentDisable = m_PlatformDBAide.GetValue_INT(TEXT("AuthentDisable"));

      // 投递结果
      EmitDataBaseEngineResult(DBO_GP_REAL_AUTH_PARAMETER, dwContextID, &CmdParameter, sizeof(CmdParameter));
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }

  return true;
}

// 等级配置
bool CDataBaseEngineSink::OnRequestLoadGrowLevelConfig(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 构造参数
    m_PlatformDBAide.ResetParameter();

    // 执行命令
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GR_LoadGrowLevelConfig"), true);

    // 执行成功
    if (lResultCode == DB_SUCCESS) {
      // 变量定义
      DBO_GP_GrowLevelConfig GrowLevelConfig;
      ZeroMemory(&GrowLevelConfig, sizeof(GrowLevelConfig));

      // 设置变量
      WORD wLevelCount = 0;
      tagGrowLevelConfig* pGrowLevelConfig = nullptr;

      // 变量定义
      while (!m_PlatformDBModule->IsRecordsetEnd()) {
        // 溢出判断
        if (GrowLevelConfig.wLevelCount >= std::size(GrowLevelConfig.GrowLevelConfig))
          break;

        // 设置变量
        pGrowLevelConfig = &GrowLevelConfig.GrowLevelConfig[GrowLevelConfig.wLevelCount++];

        // 读取数据
        pGrowLevelConfig->wLevelID = m_PlatformDBAide.GetValue_WORD(TEXT("LevelID"));
        pGrowLevelConfig->dwExperience = m_PlatformDBAide.GetValue_DWORD(TEXT("Experience"));

        // 移动记录
        m_PlatformDBModule->MoveToNext();
      }

      // 发送数据
      WORD wSendDataSize = sizeof(GrowLevelConfig) - sizeof(GrowLevelConfig.GrowLevelConfig);
      wSendDataSize += sizeof(tagGrowLevelConfig) * GrowLevelConfig.wLevelCount;
      EmitDataBaseEngineResult(DBO_GP_GROWLEVEL_CONFIG, dwContextID, &GrowLevelConfig, wSendDataSize);
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }

  return true;
}

// 查询等级
bool CDataBaseEngineSink::OnRequestQueryGrowLevelParameter(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_GrowLevelQueryInfo));
    if (wDataSize != sizeof(DBR_GP_GrowLevelQueryInfo))
      return false;

    // 请求处理
    DBR_GP_GrowLevelQueryInfo* pGrowLevelQueryInfo = (DBR_GP_GrowLevelQueryInfo*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pGrowLevelQueryInfo->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pGrowLevelQueryInfo->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@strPassword"), pGrowLevelQueryInfo->szPassword);
    m_PlatformDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_PlatformDBAide.AddParameter(TEXT("@strMachineID"), pGrowLevelQueryInfo->szMachineID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strUpgradeDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行脚本
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_QueryGrowLevel"), true);

    // 构造结构
    DBO_GP_GrowLevelParameter GrowLevelParameter;
    ZeroMemory(&GrowLevelParameter, sizeof(GrowLevelParameter));

    // 执行成功
    if (lResultCode == DB_SUCCESS) {
      // 变量定义
      if (!m_PlatformDBModule->IsRecordsetEnd()) {
        // 读取数据
        GrowLevelParameter.wCurrLevelID = m_PlatformDBAide.GetValue_WORD(TEXT("CurrLevelID"));
        GrowLevelParameter.dwExperience = m_PlatformDBAide.GetValue_DWORD(TEXT("Experience"));
        GrowLevelParameter.dwUpgradeExperience = m_PlatformDBAide.GetValue_DWORD(TEXT("UpgradeExperience"));
        GrowLevelParameter.lUpgradeRewardGold = m_PlatformDBAide.GetValue_LONGLONG(TEXT("RewardGold"));
        GrowLevelParameter.lUpgradeRewardIngot = m_PlatformDBAide.GetValue_LONGLONG(TEXT("RewardMedal"));
      }

      // 构造结构
      DBO_GP_GrowLevelUpgrade GrowLevelUpgrade;
      ZeroMemory(&GrowLevelUpgrade, sizeof(GrowLevelUpgrade));

      // 升级提示
      StringT DBVarValue;
      m_PlatformDBModule->GetParameter(TEXT("@strUpGradeDescribe"), DBVarValue);
      LSTRCPYN(GrowLevelUpgrade.szNotifyContent, DBVarValue.c_str(), std::size(GrowLevelUpgrade.szNotifyContent));
      if (GrowLevelUpgrade.szNotifyContent[0] != 0) {
        // 读取财富
        GrowLevelUpgrade.lCurrScore = m_PlatformDBAide.GetValue_LONGLONG(TEXT("Score"));
        GrowLevelUpgrade.lCurrIngot = m_PlatformDBAide.GetValue_LONGLONG(TEXT("Ingot"));

        // 发送提示
        WORD wSendDataSize = sizeof(GrowLevelUpgrade) - sizeof(GrowLevelUpgrade.szNotifyContent);
        wSendDataSize += CountStringBuffer(GrowLevelUpgrade.szNotifyContent);
        EmitDataBaseEngineResult(DBO_GP_GROWLEVEL_UPGRADE, dwContextID, &GrowLevelUpgrade, wSendDataSize);
      }
    }

    // 发送参数
    EmitDataBaseEngineResult(DBO_GP_GROWLEVEL_PARAMETER, dwContextID, &GrowLevelParameter, sizeof(GrowLevelParameter));

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 构造结构
    DBO_GP_GrowLevelParameter GrowLevelParameter;
    ZeroMemory(&GrowLevelParameter, sizeof(GrowLevelParameter));

    // 发送参数
    EmitDataBaseEngineResult(DBO_GP_GROWLEVEL_PARAMETER, dwContextID, &GrowLevelParameter, sizeof(GrowLevelParameter));

    return false;
  }
}

// 加载道具
bool CDataBaseEngineSink::OnRequestLoadGamePropertyList(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 变量定义
    WORD wPacketSize = 0;
    BYTE cbBuffer[MAX_ASYNCHRONISM_DATA];

    // 加载类型
    wPacketSize = 0;
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_LoadGamePropertyTypeItem"), true);
    DBO_GP_GamePropertyTypeItem* pGamePropertyTypeItem = nullptr;
    while (!m_PlatformDBModule->IsRecordsetEnd()) {
      // 发送信息
      if ((wPacketSize + sizeof(DBO_GP_GamePropertyTypeItem)) > (sizeof(cbBuffer) - sizeof(NTY_DataBaseEvent))) {
        EmitDataBaseEngineResult(DBO_GP_GAME_PROPERTY_TYPE_ITEM, dwContextID, cbBuffer, wPacketSize);
        wPacketSize = 0;
      }

      // 读取信息
      pGamePropertyTypeItem = (DBO_GP_GamePropertyTypeItem*)(cbBuffer + wPacketSize);
      pGamePropertyTypeItem->dwSortID = m_PlatformDBAide.GetValue_DWORD(TEXT("SortID"));
      pGamePropertyTypeItem->dwTypeID = m_PlatformDBAide.GetValue_DWORD(TEXT("TypeID"));
      m_PlatformDBAide.GetValue_String(TEXT("TypeName"), pGamePropertyTypeItem->szTypeName, std::size(pGamePropertyTypeItem->szTypeName));

      // 设置位移
      wPacketSize += sizeof(DBO_GP_GamePropertyTypeItem);

      // 移动记录
      m_PlatformDBModule->MoveToNext();
    }
    if (wPacketSize > 0)
      EmitDataBaseEngineResult(DBO_GP_GAME_PROPERTY_TYPE_ITEM, dwContextID, cbBuffer, wPacketSize);

    // 加载关系
    wPacketSize = 0;
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_LoadGamePropertyRelatItem"), true);
    DBO_GP_GamePropertyRelatItem* pGamePropertyRelatItem = nullptr;
    while (!m_PlatformDBModule->IsRecordsetEnd()) {
      // 发送信息
      if ((wPacketSize + sizeof(DBO_GP_GamePropertyRelatItem)) > (sizeof(cbBuffer) - sizeof(NTY_DataBaseEvent))) {
        EmitDataBaseEngineResult(DBO_GP_GAME_PROPERTY_RELAT_ITEM, dwContextID, cbBuffer, wPacketSize);
        wPacketSize = 0;
      }

      // 读取信息
      pGamePropertyRelatItem = (DBO_GP_GamePropertyRelatItem*)(cbBuffer + wPacketSize);
      pGamePropertyRelatItem->dwPropertyID = m_PlatformDBAide.GetValue_DWORD(TEXT("PropertyID"));
      pGamePropertyRelatItem->dwTypeID = m_PlatformDBAide.GetValue_DWORD(TEXT("TypeID"));

      // 设置位移
      wPacketSize += sizeof(DBO_GP_GamePropertyRelatItem);

      // 移动记录
      m_PlatformDBModule->MoveToNext();
    }
    if (wPacketSize > 0)
      EmitDataBaseEngineResult(DBO_GP_GAME_PROPERTY_RELAT_ITEM, dwContextID, cbBuffer, wPacketSize);

    // 加载道具
    wPacketSize = 0;
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_LoadGamePropertyItem"), true);
    DBO_GP_GamePropertyItem* pGamePropertyItem = nullptr;
    while (!m_PlatformDBModule->IsRecordsetEnd()) {
      // 发送信息
      if ((wPacketSize + sizeof(DBO_GP_GamePropertyItem)) > (sizeof(cbBuffer) - sizeof(NTY_DataBaseEvent))) {
        EmitDataBaseEngineResult(DBO_GP_GAME_PROPERTY_ITEM, dwContextID, cbBuffer, wPacketSize);
        wPacketSize = 0;
      }

      // 读取信息
      pGamePropertyItem = (DBO_GP_GamePropertyItem*)(cbBuffer + wPacketSize);
      pGamePropertyItem->dwPropertyID = m_PlatformDBAide.GetValue_DWORD(TEXT("ID"));
      pGamePropertyItem->dwPropertyKind = m_PlatformDBAide.GetValue_DWORD(TEXT("Kind"));
      pGamePropertyItem->dwExchangeRatio = m_PlatformDBAide.GetValue_DWORD(TEXT("ExchangeRatio"));
      // pGamePropertyItem->dPropertyCash=m_PlatformDBAide.GetValue_DOUBLE(TEXT("Cash"));
      // pGamePropertyItem->lPropertyGold=m_PlatformDBAide.GetValue_LONGLONG(TEXT("Gold"));
      // pGamePropertyItem->lPropertyUserMedal=m_PlatformDBAide.GetValue_LONGLONG(TEXT("UserMedal"));
      // pGamePropertyItem->lPropertyLoveLiness=m_PlatformDBAide.GetValue_LONGLONG(TEXT("LoveLiness"));
      pGamePropertyItem->cbUseArea = m_PlatformDBAide.GetValue_BYTE(TEXT("UseArea"));
      pGamePropertyItem->cbServiceArea = m_PlatformDBAide.GetValue_BYTE(TEXT("ServiceArea"));
      pGamePropertyItem->lSendLoveLiness = m_PlatformDBAide.GetValue_LONGLONG(TEXT("SendLoveLiness"));
      pGamePropertyItem->lRecvLoveLiness = m_PlatformDBAide.GetValue_LONGLONG(TEXT("RecvLoveLiness"));
      pGamePropertyItem->lUseResultsGold = m_PlatformDBAide.GetValue_LONGLONG(TEXT("UseResultsGold"));
      pGamePropertyItem->dwUseResultsValidTime = m_PlatformDBAide.GetValue_DWORD(TEXT("UseResultsValidTime"));
      pGamePropertyItem->dwUseResultsValidTimeScoreMultiple = m_PlatformDBAide.GetValue_DWORD(TEXT("UseResultsValidTimeScoreMultiple"));
      pGamePropertyItem->dwUseResultsGiftPackage = m_PlatformDBAide.GetValue_DWORD(TEXT("UseResultsGiftPackage"));
      pGamePropertyItem->cbRecommend = m_PlatformDBAide.GetValue_BYTE(TEXT("Recommend"));
      pGamePropertyItem->dwSortID = m_PlatformDBAide.GetValue_DWORD(TEXT("SortID"));
      m_PlatformDBAide.GetValue_String(TEXT("Name"), pGamePropertyItem->szName, std::size(pGamePropertyItem->szName));
      m_PlatformDBAide.GetValue_String(TEXT("RegulationsInfo"), pGamePropertyItem->szRegulationsInfo,
                                       std::size(pGamePropertyItem->szRegulationsInfo));
      // 设置位移
      wPacketSize += sizeof(DBO_GP_GamePropertyItem);

      // 移动记录
      m_PlatformDBModule->MoveToNext();
    }
    if (wPacketSize > 0)
      EmitDataBaseEngineResult(DBO_GP_GAME_PROPERTY_ITEM, dwContextID, cbBuffer, wPacketSize);

    // 加载子道具
    wPacketSize = 0;
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_LoadGamePropertySubItem"), true);
    DBO_GP_GamePropertySubItem* pGamePropertySubItem = nullptr;
    while (!m_PlatformDBModule->IsRecordsetEnd()) {
      // 发送信息
      if ((wPacketSize + sizeof(DBO_GP_GamePropertySubItem)) > (sizeof(cbBuffer) - sizeof(NTY_DataBaseEvent))) {
        EmitDataBaseEngineResult(DBO_GP_GAME_PROPERTY_SUB_ITEM, dwContextID, cbBuffer, wPacketSize);
        wPacketSize = 0;
      }

      // 读取信息
      pGamePropertySubItem = (DBO_GP_GamePropertySubItem*)(cbBuffer + wPacketSize);
      pGamePropertySubItem->dwPropertyID = m_PlatformDBAide.GetValue_DWORD(TEXT("ID"));
      pGamePropertySubItem->dwOwnerPropertyID = m_PlatformDBAide.GetValue_DWORD(TEXT("OwnerID"));
      pGamePropertySubItem->dwPropertyCount = m_PlatformDBAide.GetValue_DWORD(TEXT("Count"));
      pGamePropertySubItem->dwSortID = m_PlatformDBAide.GetValue_DWORD(TEXT("SortID"));
      // 设置位移
      wPacketSize += sizeof(DBO_GP_GamePropertySubItem);

      // 移动记录
      m_PlatformDBModule->MoveToNext();
    }
    if (wPacketSize > 0)
      EmitDataBaseEngineResult(DBO_GP_GAME_PROPERTY_SUB_ITEM, dwContextID, cbBuffer, wPacketSize);

    // 变量定义
    DBO_GP_GamePropertyListResult GamePropertyResult;
    ZeroMemory(&GamePropertyResult, sizeof(GamePropertyResult));

    // 设置变量
    GamePropertyResult.cbSuccess = TRUE;

    // 发送消息
    EmitDataBaseEngineResult(DBO_GP_GAME_PROPERTY_LIST_RESULT, dwContextID, &GamePropertyResult, sizeof(GamePropertyResult));

  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 变量定义
    DBO_GP_GamePropertyListResult GamePropertyResult;
    ZeroMemory(&GamePropertyResult, sizeof(GamePropertyResult));

    // 设置变量
    GamePropertyResult.cbSuccess = FALSE;

    // 发送消息
    EmitDataBaseEngineResult(DBO_GP_GAME_PROPERTY_LIST_RESULT, dwContextID, &GamePropertyResult, sizeof(GamePropertyResult));

    return false;
  }

  return true;
}

// 购买道具
bool CDataBaseEngineSink::OnRequestBuyGameProperty(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_PropertyBuy));
    if (wDataSize != sizeof(DBR_GP_PropertyBuy))
      return false;

    // 请求处理
    DBR_GP_PropertyBuy* pPropertyBuy = (DBR_GP_PropertyBuy*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pPropertyBuy->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pPropertyBuy->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@dwPropertyID"), pPropertyBuy->dwPropertyID);
    m_PlatformDBAide.AddParameter(TEXT("@iDiamondCount"), pPropertyBuy->dwDiamondCount);
    m_PlatformDBAide.AddParameter(TEXT("@strPassword"), pPropertyBuy->szPassword);
    m_PlatformDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_PlatformDBAide.AddParameter(TEXT("@strMachineID"), pPropertyBuy->szMachineID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);
    // 执行查询
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_BuyProperty"), true);

    if (lResultCode == DB_SUCCESS) {
      // 购买结果
      DBO_GP_PropertyBuyResult PropertyBuyResult;
      ZeroMemory(&PropertyBuyResult, sizeof(DBO_GP_PropertyBuyResult));
      PropertyBuyResult.dwUserID = pPropertyBuy->dwUserID;
      PropertyBuyResult.dwPropertyID = m_PlatformDBAide.GetValue_DWORD(TEXT("PropertyID"));
      PropertyBuyResult.dwItemCount = m_PlatformDBAide.GetValue_DWORD(TEXT("ItemCount"));
      PropertyBuyResult.lDiamond = m_PlatformDBAide.GetValue_LONGLONG(TEXT("Diamond"));
      // PropertyBuyResult.lInsureScore = m_PlatformDBAide.GetValue_LONGLONG(TEXT("Gold"));
      // PropertyBuyResult.lUserMedal = m_PlatformDBAide.GetValue_LONGLONG(TEXT("UserMedal"));
      // PropertyBuyResult.lLoveLiness = m_PlatformDBAide.GetValue_LONGLONG(TEXT("LoveLiness"));
      // PropertyBuyResult.dCash = m_PlatformDBAide.GetValue_DOUBLE(TEXT("Cash"));
      // PropertyBuyResult.cbCurrMemberOrder = m_PlatformDBAide.GetValue_BYTE(TEXT("MemberOrder"));

      // 获取提示
      StringT DBVarValue;
      m_PlatformDBAide.GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
      LSTRCPYN(PropertyBuyResult.szNotifyContent, DBVarValue.c_str(), std::size(PropertyBuyResult.szNotifyContent));

      EmitDataBaseEngineResult(DBO_GP_GAME_PROPERTY_BUY, dwContextID, &PropertyBuyResult, sizeof(DBO_GP_PropertyBuyResult));
    } else {
      // 购买结果
      DBO_GP_PropertyFailure PropertyFailure;
      ZeroMemory(&PropertyFailure, sizeof(DBO_GP_PropertyFailure));
      PropertyFailure.lErrorCode = lResultCode;

      // 获取提示
      StringT DBVarValue;
      m_PlatformDBAide.GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
      LSTRCPYN(PropertyFailure.szDescribeString, DBVarValue.c_str(), std::size(PropertyFailure.szDescribeString));

      EmitDataBaseEngineResult(DBO_GP_GAME_PROPERTY_FAILURE, dwContextID, &PropertyFailure, sizeof(PropertyFailure));
    }
    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }

  return true;
}

bool CDataBaseEngineSink::OnRequestUseProperty(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  // 效验参数
  ASSERT(wDataSize == sizeof(DBR_GP_PropertyUse));
  if (wDataSize != sizeof(DBR_GP_PropertyUse))
    return false;

  // 请求处理
  DBR_GP_PropertyUse* pPropertyUse = (DBR_GP_PropertyUse*)pData;

  DBO_GP_PropertyUse DBOPropertyUseResult;
  ZeroMemory(&DBOPropertyUseResult, sizeof(DBOPropertyUseResult));
  DBOPropertyUseResult.dwPropID = pPropertyUse->dwPropID;
  DBOPropertyUseResult.dwUserID = pPropertyUse->dwUserID;
  DBOPropertyUseResult.dwRecvUserID = pPropertyUse->dwRecvUserID;
  DBOPropertyUseResult.wPropCount = pPropertyUse->wPropCount;

  // 转化地址
  TCHAR szClientAddr[16] = TEXT("");
  BYTE* pClientAddr = (BYTE*)&pPropertyUse->dwClientAddr;
  SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

  // 构造参数
  try {
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pPropertyUse->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@dwRecvUserID"), pPropertyUse->dwRecvUserID);
    m_PlatformDBAide.AddParameter(TEXT("@dwPropID"), pPropertyUse->dwPropID);
    m_PlatformDBAide.AddParameter(TEXT("@dwPropCount"), pPropertyUse->wPropCount);
    m_PlatformDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行脚本
    // 0 成功 1道具不存在 2使用范围不对
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_UseProp"), true);
    DBOPropertyUseResult.dwHandleCode = lResultCode;
    if (lResultCode == 0) {
      DBOPropertyUseResult.lRecvLoveLiness = m_PlatformDBAide.GetValue_LONGLONG(TEXT("RecvLoveLiness"));
      DBOPropertyUseResult.lSendLoveLiness = m_PlatformDBAide.GetValue_LONGLONG(TEXT("SendLoveLiness"));
      DBOPropertyUseResult.Score = m_PlatformDBAide.GetValue_LONGLONG(TEXT("Score"));
      DBOPropertyUseResult.lUseResultsGold = m_PlatformDBAide.GetValue_INT(TEXT("UseResultsGold"));
      DBOPropertyUseResult.UseResultsValidTime = m_PlatformDBAide.GetValue_INT(TEXT("UseResultsValidTime"));
      DBOPropertyUseResult.dwScoreMultiple = m_PlatformDBAide.GetValue_INT(TEXT("UseResultsValidTimeScoreMultiple"));
      DBOPropertyUseResult.dwPropKind = m_PlatformDBAide.GetValue_INT(TEXT("Kind"));
      DBOPropertyUseResult.dwRemainderPropCount = m_PlatformDBAide.GetValue_INT(TEXT("RemainderCount"));
      DBOPropertyUseResult.cbMemberOrder = m_PlatformDBAide.GetValue_BYTE(TEXT("MemberOrder"));
      m_PlatformDBAide.GetValue_String(TEXT("Name"), DBOPropertyUseResult.szName, std::size(DBOPropertyUseResult.szName));
      SYSTEMTIME st;
      m_PlatformDBAide.GetValue_SystemTime(TEXT("UseTime"), st);
      struct tm gm = {st.wSecond, st.wMinute, st.wHour, st.wDay, st.wMonth - 1, st.wYear - 1900, st.wDayOfWeek, 0, 0};
      DBOPropertyUseResult.tUseTime = mktime(&gm);
    }

    // 获取提示
    StringT DBVarValue;
    m_PlatformDBAide.GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
    LSTRCPYN(DBOPropertyUseResult.szNotifyContent, DBVarValue.c_str(), std::size(DBOPropertyUseResult.szNotifyContent));

    EmitDataBaseEngineResult(DBO_GP_GAME_PROPERTY_USE, dwContextID, &DBOPropertyUseResult, sizeof(DBOPropertyUseResult));
    return (lResultCode == 0);
  } catch (IDataBaseException* pIException) {
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }

  return false;
}

bool CDataBaseEngineSink::OnRequestPropertyQuerySingle(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_PropertyQuerySingle));
    if (wDataSize != sizeof(DBR_GP_PropertyQuerySingle))
      return false;

    // 请求处理
    DBR_GP_PropertyQuerySingle* pPropertyBuy = (DBR_GP_PropertyQuerySingle*)pData;

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pPropertyBuy->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@dwPropertyID"), pPropertyBuy->dwPropertyID);
    m_PlatformDBAide.AddParameter(TEXT("@strPassword"), pPropertyBuy->szPassword);

    // 执行查询
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_PropertQuerySingle"), true);

    if (lResultCode == DB_SUCCESS) {
      // 购买结果
      DBO_GP_PropertyQuerySingle PropertyQuerySingle;
      ZeroMemory(&PropertyQuerySingle, sizeof(PropertyQuerySingle));
      PropertyQuerySingle.dwUserID = pPropertyBuy->dwUserID;
      PropertyQuerySingle.dwPropertyID = m_PlatformDBAide.GetValue_DWORD(TEXT("PropertyID"));
      PropertyQuerySingle.dwItemCount = m_PlatformDBAide.GetValue_DWORD(TEXT("ItemCount"));

      EmitDataBaseEngineResult(DBO_GP_PROPERTY_QUERY_SINGLE, dwContextID, &PropertyQuerySingle, sizeof(PropertyQuerySingle));
    } else {
      // 购买结果
      DBO_GP_PropertyQuerySingle PropertyQuerySingle;
      ZeroMemory(&PropertyQuerySingle, sizeof(PropertyQuerySingle));
      PropertyQuerySingle.dwUserID = pPropertyBuy->dwUserID;
      PropertyQuerySingle.dwPropertyID = pPropertyBuy->dwPropertyID;
      PropertyQuerySingle.dwItemCount = 0;

      EmitDataBaseEngineResult(DBO_GP_PROPERTY_QUERY_SINGLE, dwContextID, &PropertyQuerySingle, sizeof(PropertyQuerySingle));
    }
    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

bool CDataBaseEngineSink::OnRequestUserBackpackProperty(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_QueryBackpack));
    if (wDataSize != sizeof(DBR_GP_QueryBackpack))
      return false;

    // 请求处理
    DBR_GP_QueryBackpack* pQueryBackpack = (DBR_GP_QueryBackpack*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pQueryBackpack->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pQueryBackpack->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@dwKind"), pQueryBackpack->dwKindID);

    const DWORD dwDataHead = sizeof(DBO_GP_QueryBackpack) - sizeof(tagBackpackProperty);

    // 构造返回
    BYTE cbDataBuffer[SOCKET_TCP_PACKET] = {0};
    DBO_GP_QueryBackpack* pQueryBackpackResult = (DBO_GP_QueryBackpack*)cbDataBuffer;

    // 初始化参数
    WORD dwDataBufferSize = dwDataHead;
    pQueryBackpackResult->dwUserID = pQueryBackpack->dwUserID;
    pQueryBackpackResult->dwCount = 0;
    pQueryBackpackResult->dwStatus = 0;

    // 数据定义
    tagPropertyInfo PropertyInfo;
    ZeroMemory(&PropertyInfo, sizeof(PropertyInfo));
    int nPropertyCount = 0;

    // 执行脚本
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_QueryUserBackpack"), true);
    if (lResultCode == DB_SUCCESS) {
      int nCount = 0;
      while (true) {
        // 结束判断
        if (m_PlatformDBModule->IsRecordsetEnd())
          break;

        // 读取数据
        nPropertyCount = m_PlatformDBAide.GetValue_INT(TEXT("GoodsCount"));
        PropertyInfo.wIndex = m_PlatformDBAide.GetValue_INT(TEXT("GoodsID"));
        PropertyInfo.wKind = m_PlatformDBAide.GetValue_WORD(TEXT("Kind"));
        PropertyInfo.wUseArea = m_PlatformDBAide.GetValue_WORD(TEXT("UseArea"));
        PropertyInfo.wRecommend = m_PlatformDBAide.GetValue_INT(TEXT("Recommend"));
        PropertyInfo.lPropertyGold = m_PlatformDBAide.GetValue_LONGLONG(TEXT("Gold"));
        PropertyInfo.dPropertyCash = m_PlatformDBAide.GetValue_DOUBLE(TEXT("Cash"));
        PropertyInfo.lPropertyUserMedal = m_PlatformDBAide.GetValue_LONGLONG(TEXT("UserMedal"));
        PropertyInfo.lPropertyLoveLiness = m_PlatformDBAide.GetValue_LONGLONG(TEXT("LoveLiness"));
        PropertyInfo.lSendLoveLiness = m_PlatformDBAide.GetValue_LONGLONG(TEXT("SendLoveLiness"));
        PropertyInfo.lRecvLoveLiness = m_PlatformDBAide.GetValue_LONGLONG(TEXT("RecvLoveLiness"));
        PropertyInfo.lUseResultsGold = m_PlatformDBAide.GetValue_LONGLONG(TEXT("UseResultsGold"));
        m_PlatformDBAide.GetValue_String(TEXT("Name"), PropertyInfo.szName, std::size(PropertyInfo.szName));
        m_PlatformDBAide.GetValue_String(TEXT("RegulationsInfo"), PropertyInfo.szRegulationsInfo, std::size(PropertyInfo.szRegulationsInfo));

        // 拷贝数据
        memcpy(&pQueryBackpackResult->PropertyInfo[nCount++].Property, &PropertyInfo, sizeof(PropertyInfo));
        pQueryBackpackResult->PropertyInfo[nCount - 1].nCount = nPropertyCount;
        dwDataBufferSize += sizeof(PropertyInfo);

        // 判断数据包大小
        if (dwDataBufferSize >= SOCKET_TCP_PACKET - sizeof(PropertyInfo)) {
          // 构造提示
          CLogger::Error(TEXT("发送道具数量: {}"), pQueryBackpackResult->dwCount);

          pQueryBackpackResult->dwStatus = 0;
          EmitDataBaseEngineResult(DBO_GP_QUERY_BACKPACK, dwContextID, pQueryBackpackResult, dwDataBufferSize);
          nCount = 0;
          dwDataBufferSize = dwDataHead;
        }

        // 清理临时数据
        ZeroMemory(&PropertyInfo, sizeof(PropertyInfo));
        nPropertyCount = 0;

        // 移动记录
        m_PlatformDBModule->MoveToNext();
      }

      // 发送信息
      pQueryBackpackResult->dwCount = nCount;
      pQueryBackpackResult->dwStatus = 1;
      EmitDataBaseEngineResult(DBO_GP_QUERY_BACKPACK, dwContextID, pQueryBackpackResult, dwDataBufferSize);
    }
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

bool CDataBaseEngineSink::OnRequestPropertyBuff(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    ASSERT(wDataSize == sizeof(DBR_GP_UserPropertyBuff));
    DBR_GP_UserPropertyBuff* PropertyBuffRequest = (DBR_GP_UserPropertyBuff*)pData;

    // 变量定义
    DBO_GR_UserPropertyBuff GamePropertyBuff;
    ZeroMemory(&GamePropertyBuff, sizeof(GamePropertyBuff));
    GamePropertyBuff.dwUserID = PropertyBuffRequest->dwUserID;

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), PropertyBuffRequest->dwUserID);

    // 执行查询
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_LoadUserGameBuff"), true);

    // 读取信息
    for (WORD i = 0; i < std::size(GamePropertyBuff.PropertyBuff); i++) {
      // 结束判断
      if (m_PlatformDBModule->IsRecordsetEnd())
        break;

      // 溢出效验
      ASSERT(GamePropertyBuff.cbBuffCount < std::size(GamePropertyBuff.PropertyBuff));
      if (GamePropertyBuff.cbBuffCount >= std::size(GamePropertyBuff.PropertyBuff))
        break;

      // 读取数据
      GamePropertyBuff.cbBuffCount++;
      // GamePropertyBuff.PropertyBuff[i].dwPropID=m_PlatformDBAide.GetValue_WORD(TEXT("PropID"));
      GamePropertyBuff.PropertyBuff[i].dwKind = m_PlatformDBAide.GetValue_WORD(TEXT("KindID"));
      GamePropertyBuff.PropertyBuff[i].dwScoreMultiple = m_PlatformDBAide.GetValue_WORD(TEXT("UseResultsValidTimeScoreMultiple"));
      GamePropertyBuff.PropertyBuff[i].UseResultsValidTime = m_PlatformDBAide.GetValue_DWORD(TEXT("UseResultsValidTime"));
      m_PlatformDBAide.GetValue_String(TEXT("Name"), GamePropertyBuff.PropertyBuff[i].szName, std::size(GamePropertyBuff.PropertyBuff[i].szName));
      SYSTEMTIME st;
      m_PlatformDBAide.GetValue_SystemTime(TEXT("UseTime"), st);
      struct tm gm = {st.wSecond, st.wMinute, st.wHour, st.wDay, st.wMonth - 1, st.wYear - 1900, st.wDayOfWeek, 0, 0};
      GamePropertyBuff.PropertyBuff[i].tUseTime = mktime(&gm);

      // 移动记录
      m_PlatformDBModule->MoveToNext();
    }
    // 发送信息
    WORD wHeadSize = sizeof(GamePropertyBuff) - sizeof(GamePropertyBuff.PropertyBuff);
    WORD wDataSize = GamePropertyBuff.cbBuffCount * sizeof(GamePropertyBuff.PropertyBuff[0]);
    EmitDataBaseEngineResult(DBO_GP_USER_PROPERTY_BUFF, dwContextID, &GamePropertyBuff, wHeadSize + wDataSize);
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 变量定义
    DBO_GR_UserPropertyBuff GamePropertyBuff;
    ZeroMemory(&GamePropertyBuff, sizeof(GamePropertyBuff));

    // 构造变量
    GamePropertyBuff.cbBuffCount = 0L;

    // 发送信息
    WORD wHeadSize = sizeof(GamePropertyBuff) - sizeof(GamePropertyBuff.PropertyBuff);
    EmitDataBaseEngineResult(DBO_GP_USER_PROPERTY_BUFF, dwContextID, &GamePropertyBuff, wHeadSize);

    return false;
  }

  return true;
}

bool CDataBaseEngineSink::OnRequestPropertyPresent(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    ASSERT(wDataSize == sizeof(DBR_GP_PropertyPresent));
    DBR_GP_PropertyPresent* PropertyPresentRequest = (DBR_GP_PropertyPresent*)pData;

    // 变量定义
    DBO_GP_PropertyPresent PropertyPresent = {0};
    PropertyPresent.dwUserID = PropertyPresentRequest->dwUserID;
    PropertyPresent.dwRecvGameID = PropertyPresentRequest->dwRecvGameID;
    PropertyPresent.dwPropID = PropertyPresentRequest->dwPropID;
    PropertyPresent.wPropCount = PropertyPresentRequest->wPropCount;
    PropertyPresent.wType = PropertyPresentRequest->wType; // 0昵称  1 GameID
    LSTRCPYN(PropertyPresent.szRecvNickName, PropertyPresentRequest->szRecvNickName, std::size(PropertyPresent.szRecvNickName));

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&PropertyPresentRequest->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), PropertyPresentRequest->dwUserID);
    if (PropertyPresentRequest->wType == 0) // 按照昵称赠送
    {
      m_PlatformDBAide.AddParameter(TEXT("@strReceiverNickName"), PropertyPresentRequest->szRecvNickName);
    } else if (PropertyPresentRequest->wType == 1) // 按照 GameID
    {
      m_PlatformDBAide.AddParameter(TEXT("@dwReceiverGameID"), PropertyPresentRequest->dwRecvGameID);
    }
    m_PlatformDBAide.AddParameter(TEXT("@dwPropID"), PropertyPresentRequest->dwPropID);
    m_PlatformDBAide.AddParameter(TEXT("@dwPropCount"), PropertyPresentRequest->wPropCount);
    m_PlatformDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);

    // 输出参数
    TCHAR szDescribeString[64] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    LONG lResultCode = -1;
    if (PropertyPresentRequest->wType == 0) {
      lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_UserSendPresentByNickName"), true);
    } else if (PropertyPresentRequest->wType == 1) {
      lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_UserSendPresentByID"), true);
    }

    // 返回码
    PropertyPresent.nHandleCode = lResultCode;

    // 获取参数
    StringT DBVarValue;
    m_PlatformDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
    LSTRCPYN(PropertyPresent.szNotifyContent, DBVarValue.c_str(), std::size(PropertyPresent.szNotifyContent));

    // 发送信息
    EmitDataBaseEngineResult(DBO_GP_PROPERTY_PRESENT, dwContextID, &PropertyPresent, sizeof(PropertyPresent));
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    DBR_GP_PropertyPresent* PropertyPresentRequest = (DBR_GP_PropertyPresent*)pData;
    // 变量定义
    DBO_GP_PropertyPresent PropertyPresent = {0};
    PropertyPresent.dwUserID = PropertyPresentRequest->dwUserID;
    PropertyPresent.dwRecvGameID = PropertyPresentRequest->dwRecvGameID;
    PropertyPresent.dwPropID = PropertyPresentRequest->dwPropID;
    PropertyPresent.wPropCount = PropertyPresentRequest->wPropCount;
    PropertyPresent.wType = PropertyPresentRequest->wType; // 0昵称  1 GameID
    LSTRCPYN(PropertyPresent.szRecvNickName, PropertyPresentRequest->szRecvNickName, std::size(PropertyPresent.szRecvNickName));
    // 构造变量
    PropertyPresent.nHandleCode = -1;
    LSTRCPYN(PropertyPresent.szNotifyContent, TEXT("赠送道具时 数据库操作异常"), std::size(PropertyPresent.szNotifyContent));

    // 发送信息
    EmitDataBaseEngineResult(DBO_GP_PROPERTY_PRESENT, dwContextID, &PropertyPresent, sizeof(PropertyPresent));
    return false;
  }

  return true;
}

// 查询赠送
bool CDataBaseEngineSink::OnRequestQuerySendPresent(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    ASSERT(wDataSize == sizeof(DBR_GP_QuerySendPresent));
    DBR_GP_QuerySendPresent* pQuerySendPresent = (DBR_GP_QuerySendPresent*)pData;

    // 变量定义
    DBO_GP_QuerySendPresent QuerySendPresenResult;
    ZeroMemory(&QuerySendPresenResult, sizeof(QuerySendPresenResult));

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pQuerySendPresent->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pQuerySendPresent->dwUserID);

    // 执行查询
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_QuerySendPresent"), true);
    WORD wHeadSize = sizeof(QuerySendPresenResult) - sizeof(QuerySendPresenResult.Present);

    if (lResultCode == 0) {
      // 读取信息
      for (WORD i = 0;; i++) {
        // 结束判断
        if (m_PlatformDBModule->IsRecordsetEnd())
          break;

        // 包大小效验
        ASSERT(QuerySendPresenResult.wPresentCount < std::size(QuerySendPresenResult.Present));
        if (QuerySendPresenResult.wPresentCount >= std::size(QuerySendPresenResult.Present)) {
          WORD wDataSize = QuerySendPresenResult.wPresentCount * sizeof(QuerySendPresenResult.Present[0]);
          EmitDataBaseEngineResult(DBO_GP_QUERY_SEND_PRESENT, dwContextID, &QuerySendPresenResult, wHeadSize + wDataSize);
          i = 0;
          QuerySendPresenResult.wPresentCount = 0;
        }

        // 读取数据
        QuerySendPresenResult.wPresentCount++;
        QuerySendPresenResult.Present[i].dwUserID = m_PlatformDBAide.GetValue_WORD(TEXT("UserID"));
        QuerySendPresenResult.Present[i].dwRecvUserID = m_PlatformDBAide.GetValue_WORD(TEXT("ReceiverUserID"));
        QuerySendPresenResult.Present[i].dwPropID = m_PlatformDBAide.GetValue_WORD(TEXT("PropID"));
        QuerySendPresenResult.Present[i].wPropCount = m_PlatformDBAide.GetValue_WORD(TEXT("PropCount"));
        m_PlatformDBAide.GetValue_String(TEXT("Name"), QuerySendPresenResult.Present[i].szPropName,
                                         std::size(QuerySendPresenResult.Present[i].szPropName));
        SYSTEMTIME st;
        m_PlatformDBAide.GetValue_SystemTime(TEXT("SendTime"), st);
        struct tm gm = {st.wSecond, st.wMinute, st.wHour, st.wDay, st.wMonth - 1, st.wYear - 1900, st.wDayOfWeek, 0, 0};
        QuerySendPresenResult.Present[i].tSendTime = mktime(&gm);

        // 移动记录
        m_PlatformDBModule->MoveToNext();
      }
    }
    // 发送信息
    wDataSize = QuerySendPresenResult.wPresentCount * sizeof(QuerySendPresenResult.Present[0]);
    EmitDataBaseEngineResult(DBO_GP_QUERY_SEND_PRESENT, dwContextID, &QuerySendPresenResult, wHeadSize + wDataSize);
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 变量定义
    // 变量定义
    DBO_GP_QuerySendPresent QuerySendPresenResult;
    ZeroMemory(&QuerySendPresenResult, sizeof(QuerySendPresenResult));

    // 构造变量
    QuerySendPresenResult.wPresentCount = 0;

    // 发送信息
    WORD wHeadSize = sizeof(QuerySendPresenResult) - sizeof(QuerySendPresenResult.Present);
    EmitDataBaseEngineResult(DBO_GP_QUERY_SEND_PRESENT, dwContextID, &QuerySendPresenResult, wHeadSize);

    return false;
  }

  return true;
}

bool CDataBaseEngineSink::OnRequestGetSendPresent(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    ASSERT(wDataSize == sizeof(DBR_GP_GetSendPresent));
    DBR_GP_GetSendPresent* pGetSendPresent = (DBR_GP_GetSendPresent*)pData;

    // 变量定义
    DBO_GP_GetSendPresent GetSendPresenResult;
    ZeroMemory(&GetSendPresenResult, sizeof(GetSendPresenResult));

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pGetSendPresent->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pGetSendPresent->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@szPassword"), pGetSendPresent->szPassword);
    m_PlatformDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);

    // 执行查询
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_GetSendPresent"), true);
    WORD wHeadSize = sizeof(GetSendPresenResult) - sizeof(GetSendPresenResult.Present);

    if (lResultCode == 0) {
      // 读取信息
      for (WORD i = 0;; i++) {
        // 结束判断
        if (m_PlatformDBModule->IsRecordsetEnd())
          break;

        // 包大小效验
        ASSERT(GetSendPresenResult.wPresentCount < std::size(GetSendPresenResult.Present));
        if (GetSendPresenResult.wPresentCount >= std::size(GetSendPresenResult.Present)) {
          WORD wDataSize = GetSendPresenResult.wPresentCount * sizeof(GetSendPresenResult.Present[0]);
          EmitDataBaseEngineResult(DBO_GP_GET_SEND_PRESENT, dwContextID, &GetSendPresenResult, wHeadSize + wDataSize);
          i = 0;
          GetSendPresenResult.wPresentCount = 0;
        }

        // 读取数据
        GetSendPresenResult.wPresentCount++;
        GetSendPresenResult.Present[i].dwUserID = m_PlatformDBAide.GetValue_WORD(TEXT("UserID"));
        GetSendPresenResult.Present[i].dwRecvUserID = m_PlatformDBAide.GetValue_WORD(TEXT("ReceiverUserID"));
        GetSendPresenResult.Present[i].dwPropID = m_PlatformDBAide.GetValue_WORD(TEXT("PropID"));
        GetSendPresenResult.Present[i].wPropCount = m_PlatformDBAide.GetValue_WORD(TEXT("PropCount"));
        m_PlatformDBAide.GetValue_String(TEXT("Name"), GetSendPresenResult.Present[i].szPropName,
                                         std::size(GetSendPresenResult.Present[i].szPropName));
        SYSTEMTIME st;
        m_PlatformDBAide.GetValue_SystemTime(TEXT("SendTime"), st);
        struct tm gm = {st.wSecond, st.wMinute, st.wHour, st.wDay, st.wMonth - 1, st.wYear - 1900, st.wDayOfWeek, 0, 0};
        GetSendPresenResult.Present[i].tSendTime = mktime(&gm);

        // 移动记录
        m_PlatformDBModule->MoveToNext();
      }
    }
    // 发送信息
    wDataSize = GetSendPresenResult.wPresentCount * sizeof(GetSendPresenResult.Present[0]);
    EmitDataBaseEngineResult(DBO_GP_GET_SEND_PRESENT, dwContextID, &GetSendPresenResult, wHeadSize + wDataSize);
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 变量定义
    // 变量定义
    DBO_GP_GetSendPresent GetSendPresenResult;
    ZeroMemory(&GetSendPresenResult, sizeof(GetSendPresenResult));

    // 构造变量
    GetSendPresenResult.wPresentCount = 0;

    // 发送信息
    WORD wHeadSize = sizeof(GetSendPresenResult) - sizeof(GetSendPresenResult.Present);
    EmitDataBaseEngineResult(DBO_GP_GET_SEND_PRESENT, dwContextID, &GetSendPresenResult, wHeadSize);

    return false;
  }
  return true;
}
// 会员参数
bool CDataBaseEngineSink::OnRequestMemberLoadParameter(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 构造参数
    m_PlatformDBAide.ResetParameter();

    // 执行命令
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_LoadMemberParameter"), true);

    // 执行成功
    if (lResultCode == DB_SUCCESS) {
      // 变量定义
      DBO_GP_MemberParameter MemberParameter;
      ZeroMemory(&MemberParameter, sizeof(MemberParameter));

      // 设置变量
      WORD wMemberCount = 0;
      tagMemberParameterNew* pMemberParameter = nullptr;

      // 变量定义
      while (!m_PlatformDBModule->IsRecordsetEnd()) {
        // 溢出判断
        if (MemberParameter.wMemberCount >= std::size(MemberParameter.MemberParameter))
          break;

        // 设置变量
        pMemberParameter = &MemberParameter.MemberParameter[MemberParameter.wMemberCount++];

        // 读取数据
        pMemberParameter->cbMemberOrder = m_PlatformDBAide.GetValue_BYTE(TEXT("MemberOrder"));
        m_PlatformDBAide.GetValue_String(TEXT("MemberName"), pMemberParameter->szMemberName, std::size(pMemberParameter->szMemberName));
        pMemberParameter->dwMemberRight = m_PlatformDBAide.GetValue_DWORD(TEXT("UserRight"));
        pMemberParameter->dwMemberTask = m_PlatformDBAide.GetValue_DWORD(TEXT("TaskRate"));
        pMemberParameter->dwMemberShop = m_PlatformDBAide.GetValue_DWORD(TEXT("ShopRate"));
        pMemberParameter->dwMemberInsure = m_PlatformDBAide.GetValue_DWORD(TEXT("InsureRate"));
        pMemberParameter->dwMemberDayPresent = m_PlatformDBAide.GetValue_DWORD(TEXT("DayPresent"));
        pMemberParameter->dwMemberDayGiftID = m_PlatformDBAide.GetValue_DWORD(TEXT("DayGiftID"));
        // 移动记录
        m_PlatformDBModule->MoveToNext();
      }

      // 发送数据
      WORD wSendDataSize = sizeof(MemberParameter) - sizeof(MemberParameter.MemberParameter);
      wSendDataSize += sizeof(tagMemberParameterNew) * MemberParameter.wMemberCount;
      EmitDataBaseEngineResult(DBO_GP_MEMBER_PARAMETER, dwContextID, &MemberParameter, wSendDataSize);
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }

  return true;
}

// 会员查询
bool CDataBaseEngineSink::OnRequestMemberQueryInfo(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_MemberQueryInfo));
    if (wDataSize != sizeof(DBR_GP_MemberQueryInfo))
      return false;

    // 提取数据
    DBR_GP_MemberQueryInfo* pDBRMemberInfo = (DBR_GP_MemberQueryInfo*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pDBRMemberInfo->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pDBRMemberInfo->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@strPassword"), pDBRMemberInfo->szPassword);
    m_PlatformDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_PlatformDBAide.AddParameter(TEXT("@strMachineID"), pDBRMemberInfo->szMachineID);

    // 执行脚本
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_MemberQueryInfo"), true);

    // 变量定义
    DBO_GP_MemberQueryInfoResult pDBOMemberInfoResult;
    ZeroMemory(&pDBOMemberInfoResult, sizeof(pDBOMemberInfoResult));

    // 读取赠送
    if (lResultCode == DB_SUCCESS && !m_PlatformDBModule->IsRecordsetEnd()) {
      pDBOMemberInfoResult.bPresent = (m_PlatformDBAide.GetValue_BYTE(TEXT("Present")) == TRUE);
      pDBOMemberInfoResult.bGift = (m_PlatformDBAide.GetValue_BYTE(TEXT("Gift")) == TRUE);
    }

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pDBRMemberInfo->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@strPassword"), pDBRMemberInfo->szPassword);
    m_PlatformDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_PlatformDBAide.AddParameter(TEXT("@strMachineID"), pDBRMemberInfo->szMachineID);

    // 执行命令
    lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_MemberGiftQuery"), true);

    // 执行成功
    if (lResultCode == DB_SUCCESS) {
      // 设置变量
      WORD wMemberCount = 0;
      tagGiftPropertyInfo* pMemberParameter = nullptr;

      // 变量定义
      while (!m_PlatformDBModule->IsRecordsetEnd()) {
        // 溢出判断
        if (pDBOMemberInfoResult.GiftSubCount >= std::size(pDBOMemberInfoResult.GiftSub))
          break;

        // 设置变量
        pMemberParameter = &pDBOMemberInfoResult.GiftSub[pDBOMemberInfoResult.GiftSubCount++];
        // 读取数据
        pMemberParameter->wPropertyID = m_PlatformDBAide.GetValue_WORD(TEXT("ID"));
        pMemberParameter->wPropertyCount = m_PlatformDBAide.GetValue_WORD(TEXT("Count"));
        // 移动记录
        m_PlatformDBModule->MoveToNext();
      }
    }

    // 计算大小
    WORD wSendDataSize = sizeof(pDBOMemberInfoResult) - sizeof(pDBOMemberInfoResult.GiftSub);
    wSendDataSize += sizeof(tagGiftPropertyInfo) * (WORD)(pDBOMemberInfoResult.GiftSubCount);

    EmitDataBaseEngineResult(DBO_GP_MEMBER_QUERY_INFO_RESULT, dwContextID, &pDBOMemberInfoResult, wSendDataSize);

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }

  return true;
}

// 会员赠送
bool CDataBaseEngineSink::OnRequestMemberDayPresent(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_MemberDayPresent));
    if (wDataSize != sizeof(DBR_GP_MemberDayPresent))
      return false;

    // 提取数据
    DBR_GP_MemberDayPresent* pDBRMemberInfo = (DBR_GP_MemberDayPresent*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pDBRMemberInfo->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pDBRMemberInfo->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@strPassword"), pDBRMemberInfo->szPassword);
    m_PlatformDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_PlatformDBAide.AddParameter(TEXT("@strMachineID"), pDBRMemberInfo->szMachineID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strNotifyContent"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行脚本
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_MemberDayPresent"), true);

    // 变量定义
    DBO_GP_MemberDayPresentResult pDBOMemberInfoResult;
    ZeroMemory(&pDBOMemberInfoResult, sizeof(pDBOMemberInfoResult));

    // 获取返回
    pDBOMemberInfoResult.bSuccessed = (lResultCode == DB_SUCCESS);

    // 读取赠送
    if (lResultCode == DB_SUCCESS) {
      pDBOMemberInfoResult.lGameScore = m_PlatformDBAide.GetValue_LONGLONG(TEXT("Score"));
    }

    // 获取参数
    StringT DBVarValue;
    m_PlatformDBModule->GetParameter(TEXT("@strNotifyContent"), DBVarValue);
    LSTRCPYN(pDBOMemberInfoResult.szNotifyContent, DBVarValue.c_str(), std::size(pDBOMemberInfoResult.szNotifyContent));

    // 发送结果
    WORD wSendSize = sizeof(pDBOMemberInfoResult) - sizeof(pDBOMemberInfoResult.szNotifyContent);
    wSendSize += CountStringBuffer(pDBOMemberInfoResult.szNotifyContent);
    EmitDataBaseEngineResult(DBO_GP_MEMBER_DAY_PRESENT_RESULT, dwContextID, &pDBOMemberInfoResult, wSendSize);
    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }

  return true;
}

// 会员礼包
bool CDataBaseEngineSink::OnRequestMemberDayGift(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_MemberDayGift));
    if (wDataSize != sizeof(DBR_GP_MemberDayGift))
      return false;

    // 提取数据
    DBR_GP_MemberDayGift* pDBRMemberInfo = (DBR_GP_MemberDayGift*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pDBRMemberInfo->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pDBRMemberInfo->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@strPassword"), pDBRMemberInfo->szPassword);
    m_PlatformDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_PlatformDBAide.AddParameter(TEXT("@strMachineID"), pDBRMemberInfo->szMachineID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strNotifyContent"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行脚本
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_MemberDayGift"), true);

    // 变量定义
    DBO_GP_MemberDayGiftResult pDBOMemberInfoResult;
    ZeroMemory(&pDBOMemberInfoResult, sizeof(pDBOMemberInfoResult));

    // 获取返回
    pDBOMemberInfoResult.bSuccessed = (lResultCode == DB_SUCCESS);
    // 获取参数
    StringT DBVarValue;
    m_PlatformDBModule->GetParameter(TEXT("@strNotifyContent"), DBVarValue);
    LSTRCPYN(pDBOMemberInfoResult.szNotifyContent, DBVarValue.c_str(), std::size(pDBOMemberInfoResult.szNotifyContent));

    // 发送结果
    WORD wSendSize = sizeof(pDBOMemberInfoResult) - sizeof(pDBOMemberInfoResult.szNotifyContent);
    wSendSize += CountStringBuffer(pDBOMemberInfoResult.szNotifyContent);
    EmitDataBaseEngineResult(DBO_GP_MEMBER_DAY_GIFT_RESULT, dwContextID, &pDBOMemberInfoResult, wSendSize);
    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }

  return true;
}

// 购买会员
bool CDataBaseEngineSink::OnRequestPurchaseMember(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_PurchaseMember));
    if (wDataSize != sizeof(DBR_GP_PurchaseMember))
      return false;

    // 请求处理
    DBR_GP_PurchaseMember* pPurchaseMember = (DBR_GP_PurchaseMember*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pPurchaseMember->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_TreasureDBAide.ResetParameter();
    m_TreasureDBAide.AddParameter(TEXT("@dwUserID"), pPurchaseMember->dwUserID);
    m_TreasureDBAide.AddParameter(TEXT("@cbMemberOrder"), pPurchaseMember->cbMemberOrder);
    m_TreasureDBAide.AddParameter(TEXT("@PurchaseTime"), pPurchaseMember->wPurchaseTime);
    m_TreasureDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_TreasureDBAide.AddParameter(TEXT("@strMachineID"), pPurchaseMember->szMachineID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_TreasureDBAide.AddParameterOutput(TEXT("@strNotifyContent"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行脚本
    LONG lResultCode = m_TreasureDBAide.ExecuteProcess(TEXT("GSP_GR_PurchaseMember"), true);

    // 构造结构
    DBO_GP_PurchaseResult PurchaseResult;
    ZeroMemory(&PurchaseResult, sizeof(PurchaseResult));

    // 执行成功
    if (lResultCode == DB_SUCCESS) {
      // 设置变量
      PurchaseResult.bSuccessed = true;

      // 变量定义
      if (!m_TreasureDBModule->IsRecordsetEnd()) {
        // 读取数据
        PurchaseResult.lCurrScore = m_TreasureDBAide.GetValue_LONGLONG(TEXT("CurrScore"));
        PurchaseResult.dCurrBeans = m_TreasureDBAide.GetValue_DOUBLE(TEXT("CurrBeans"));
        PurchaseResult.cbMemberOrder = m_TreasureDBAide.GetValue_BYTE(TEXT("MemberOrder"));
      }
    }

    // 获取提示
    StringT DBVarValue;
    m_TreasureDBModule->GetParameter(TEXT("@strNotifyContent"), DBVarValue);
    LSTRCPYN(PurchaseResult.szNotifyContent, DBVarValue.c_str(), std::size(PurchaseResult.szNotifyContent));

    // 计算大小
    WORD wSendDataSize = sizeof(PurchaseResult) - sizeof(PurchaseResult.szNotifyContent);
    wSendDataSize += CountStringBuffer(PurchaseResult.szNotifyContent);

    // 发送参数
    EmitDataBaseEngineResult(DBO_GP_PURCHASE_RESULT, dwContextID, &PurchaseResult, wSendDataSize);

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 构造结构
    DBO_GP_PurchaseResult PurchaseResult;
    ZeroMemory(&PurchaseResult, sizeof(PurchaseResult));

    // 设置变量
    LSTRCPYN(PurchaseResult.szNotifyContent, TEXT("数据库异常，请稍后再试！"), std::size(PurchaseResult.szNotifyContent));

    // 计算大小
    WORD wSendDataSize = sizeof(PurchaseResult) - sizeof(PurchaseResult.szNotifyContent);
    wSendDataSize += CountStringBuffer(PurchaseResult.szNotifyContent);

    // 发送参数
    EmitDataBaseEngineResult(DBO_GP_PURCHASE_RESULT, dwContextID, &PurchaseResult, wSendDataSize);

    return false;
  }
}

// 兑换游戏币
bool CDataBaseEngineSink::OnRequestExchangeScoreByIngot(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_ExchangeScoreByIngot));
    if (wDataSize != sizeof(DBR_GP_ExchangeScoreByIngot))
      return false;

    // 请求处理
    DBR_GP_ExchangeScoreByIngot* pExchangeScore = (DBR_GP_ExchangeScoreByIngot*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pExchangeScore->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_TreasureDBAide.ResetParameter();
    m_TreasureDBAide.AddParameter(TEXT("@dwUserID"), pExchangeScore->dwUserID);
    m_TreasureDBAide.AddParameter(TEXT("@ExchangeIngot"), pExchangeScore->lExchangeIngot);
    m_TreasureDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_TreasureDBAide.AddParameter(TEXT("@strMachineID"), pExchangeScore->szMachineID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_TreasureDBAide.AddParameterOutput(TEXT("@strNotifyContent"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行脚本
    LONG lResultCode = m_TreasureDBAide.ExecuteProcess(TEXT("GSP_GR_ExchangeScoreByIngot"), true);

    // 构造结构
    DBO_GP_ExchangeResult ExchangeResult;
    ZeroMemory(&ExchangeResult, sizeof(ExchangeResult));

    // 执行成功
    if (lResultCode == DB_SUCCESS) {
      // 设置变量
      ExchangeResult.bSuccessed = true;

      // 变量定义
      if (!m_TreasureDBModule->IsRecordsetEnd()) {
        // 读取数据
        ExchangeResult.lCurrScore = m_TreasureDBAide.GetValue_LONGLONG(TEXT("CurrScore"));
        ExchangeResult.lCurrIngot = m_TreasureDBAide.GetValue_LONGLONG(TEXT("CurrIngot"));
        ExchangeResult.dCurrBeans = m_TreasureDBAide.GetValue_DOUBLE(TEXT("CurrBeans"));
      }
    }

    // 提示内容
    StringT DBVarValue;
    m_TreasureDBModule->GetParameter(TEXT("@strNotifyContent"), DBVarValue);
    LSTRCPYN(ExchangeResult.szNotifyContent, DBVarValue.c_str(), std::size(ExchangeResult.szNotifyContent));

    // 计算大小
    WORD wSendDataSize = sizeof(ExchangeResult) - sizeof(ExchangeResult.szNotifyContent);
    wSendDataSize += CountStringBuffer(ExchangeResult.szNotifyContent);

    // 发送参数
    EmitDataBaseEngineResult(DBO_GP_EXCHANGE_RESULT, dwContextID, &ExchangeResult, wSendDataSize);

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 构造结构
    DBO_GP_ExchangeResult ExchangeResult;
    ZeroMemory(&ExchangeResult, sizeof(ExchangeResult));

    // 设置变量
    LSTRCPYN(ExchangeResult.szNotifyContent, TEXT("数据库异常，请稍后再试！"), std::size(ExchangeResult.szNotifyContent));

    // 计算大小
    WORD wSendDataSize = sizeof(ExchangeResult) - sizeof(ExchangeResult.szNotifyContent);
    wSendDataSize += CountStringBuffer(ExchangeResult.szNotifyContent);

    // 发送参数
    EmitDataBaseEngineResult(DBO_GP_EXCHANGE_RESULT, dwContextID, &ExchangeResult, wSendDataSize);

    return false;
  }
}

// 兑换游戏币
bool CDataBaseEngineSink::OnRequestExchangeScoreByBeans(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_ExchangeScoreByBeans));
    if (wDataSize != sizeof(DBR_GP_ExchangeScoreByBeans))
      return false;

    // 请求处理
    DBR_GP_ExchangeScoreByBeans* pExchangeScore = (DBR_GP_ExchangeScoreByBeans*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pExchangeScore->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_TreasureDBAide.ResetParameter();
    m_TreasureDBAide.AddParameter(TEXT("@dwUserID"), pExchangeScore->dwUserID);
    m_TreasureDBAide.AddParameter(TEXT("@ExchangeBeans"), pExchangeScore->dExchangeBeans);
    m_TreasureDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_TreasureDBAide.AddParameter(TEXT("@strMachineID"), pExchangeScore->szMachineID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_TreasureDBAide.AddParameterOutput(TEXT("@strNotifyContent"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行脚本
    LONG lResultCode = m_TreasureDBAide.ExecuteProcess(TEXT("GSP_GR_ExchangeScoreByBeans"), true);

    // 构造结构
    DBO_GP_ExchangeResult ExchangeResult;
    ZeroMemory(&ExchangeResult, sizeof(ExchangeResult));

    // 执行成功
    if (lResultCode == DB_SUCCESS) {
      // 设置变量
      ExchangeResult.bSuccessed = true;

      // 变量定义
      if (!m_TreasureDBModule->IsRecordsetEnd()) {
        // 读取数据
        ExchangeResult.lCurrScore = m_TreasureDBAide.GetValue_LONGLONG(TEXT("CurrScore"));
        // ExchangeResult.lCurrIngot = m_TreasureDBAide.GetValue_LONGLONG(TEXT("CurrIngot"));
        ExchangeResult.dCurrBeans = m_TreasureDBAide.GetValue_DOUBLE(TEXT("CurrBeans"));
      }
    }

    // 提示内容
    StringT DBVarValue;
    m_TreasureDBModule->GetParameter(TEXT("@strNotifyContent"), DBVarValue);
    LSTRCPYN(ExchangeResult.szNotifyContent, DBVarValue.c_str(), std::size(ExchangeResult.szNotifyContent));

    // 计算大小
    WORD wSendDataSize = sizeof(ExchangeResult) - sizeof(ExchangeResult.szNotifyContent);
    wSendDataSize += CountStringBuffer(ExchangeResult.szNotifyContent);

    // 发送参数
    EmitDataBaseEngineResult(DBO_GP_EXCHANGE_RESULT, dwContextID, &ExchangeResult, wSendDataSize);

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 构造结构
    DBO_GP_ExchangeResult ExchangeResult;
    ZeroMemory(&ExchangeResult, sizeof(ExchangeResult));

    // 设置变量
    LSTRCPYN(ExchangeResult.szNotifyContent, TEXT("数据库异常，请稍后再试！"), std::size(ExchangeResult.szNotifyContent));

    // 计算大小
    WORD wSendDataSize = sizeof(ExchangeResult) - sizeof(ExchangeResult.szNotifyContent);
    wSendDataSize += CountStringBuffer(ExchangeResult.szNotifyContent);

    // 发送参数
    EmitDataBaseEngineResult(DBO_GP_EXCHANGE_RESULT, dwContextID, &ExchangeResult, wSendDataSize);

    return false;
  }
}

// 兑换游戏币
bool CDataBaseEngineSink::OnRequestRoomCardExchangeToScore(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_GP_ExchangeScoreByRoomCard));
    if (wDataSize != sizeof(DBR_GP_ExchangeScoreByRoomCard))
      return false;

    // 请求处理
    DBR_GP_ExchangeScoreByRoomCard* pExchangeScore = (DBR_GP_ExchangeScoreByRoomCard*)pData;

    // 转化地址
    TCHAR szClientAddr[16] = TEXT("");
    BYTE* pClientAddr = (BYTE*)&pExchangeScore->dwClientAddr;
    SnprintfT(szClientAddr, std::size(szClientAddr), TEXT("%d.%d.%d.%d"), pClientAddr[0], pClientAddr[1], pClientAddr[2], pClientAddr[3]);

    // 构造参数
    m_TreasureDBAide.ResetParameter();
    m_TreasureDBAide.AddParameter(TEXT("@dwUserID"), pExchangeScore->dwUserID);
    m_TreasureDBAide.AddParameter(TEXT("@ExchangeRoomCard"), pExchangeScore->lExchangeRoomCard);
    m_TreasureDBAide.AddParameter(TEXT("@strClientIP"), szClientAddr);
    m_TreasureDBAide.AddParameter(TEXT("@strMachineID"), pExchangeScore->szMachineID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_TreasureDBAide.AddParameterOutput(TEXT("@strNotifyContent"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行脚本
    LONG lResultCode = m_TreasureDBAide.ExecuteProcess(TEXT("GSP_GR_ExchangeRoomCardByScore"), true);

    // 构造结构
    DBO_GP_RoomCardExchangeResult ExchangeResult;
    ZeroMemory(&ExchangeResult, sizeof(ExchangeResult));

    // 执行成功
    if (lResultCode == DB_SUCCESS) {
      // 设置变量
      ExchangeResult.bSuccessed = true;

      // 变量定义
      if (!m_TreasureDBModule->IsRecordsetEnd()) {
        // 读取数据
        ExchangeResult.lCurrScore = m_TreasureDBAide.GetValue_LONGLONG(TEXT("CurrScore"));
        ExchangeResult.lCurrRoomCard = m_TreasureDBAide.GetValue_LONGLONG(TEXT("Diamond"));
      }
    }

    // 提示内容
    StringT DBVarValue;
    m_TreasureDBModule->GetParameter(TEXT("@strNotifyContent"), DBVarValue);
    LSTRCPYN(ExchangeResult.szNotifyContent, DBVarValue.c_str(), std::size(ExchangeResult.szNotifyContent));

    // 计算大小
    WORD wSendDataSize = sizeof(ExchangeResult) - sizeof(ExchangeResult.szNotifyContent);
    wSendDataSize += CountStringBuffer(ExchangeResult.szNotifyContent);

    // 发送参数
    EmitDataBaseEngineResult(DBO_GP_ROOM_CARD_EXCHANGE_RESULT, dwContextID, &ExchangeResult, wSendDataSize);

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    // 构造结构
    DBO_GP_RoomCardExchangeResult ExchangeResult;
    ZeroMemory(&ExchangeResult, sizeof(ExchangeResult));
    ExchangeResult.bSuccessed = false;
    // 设置变量
    LSTRCPYN(ExchangeResult.szNotifyContent, TEXT("数据库异常，请稍后再试！"), std::size(ExchangeResult.szNotifyContent));

    // 计算大小
    WORD wSendDataSize = sizeof(ExchangeResult) - sizeof(ExchangeResult.szNotifyContent);
    wSendDataSize += CountStringBuffer(ExchangeResult.szNotifyContent);

    // 发送参数
    EmitDataBaseEngineResult(DBO_GP_ROOM_CARD_EXCHANGE_RESULT, dwContextID, &ExchangeResult, wSendDataSize);

    return false;
  }
}

// 视频信息
bool CDataBaseEngineSink::OnRequestVideoInfo(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_MB_QueryVideoInfo));
    if (wDataSize != sizeof(DBR_MB_QueryVideoInfo))
      return false;

    // 请求处理
    DBR_MB_QueryVideoInfo* pQueryVideo = (DBR_MB_QueryVideoInfo*)pData;

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@cbQueryType"), pQueryVideo->iQueryType);
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pQueryVideo->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@dwPlayBack"), pQueryVideo->dwPlayBack);
    m_PlatformDBAide.AddParameter(TEXT("@dwPersonalRoomID"), pQueryVideo->dwPersonalRoomID);
    m_PlatformDBAide.AddParameter(TEXT("@wIndexBegin"), pQueryVideo->wIndexBegin);
    m_PlatformDBAide.AddParameter(TEXT("@wIndexEnd"), pQueryVideo->wIndexEnd);
    m_PlatformDBAide.AddParameter(TEXT("@dwClubID"), pQueryVideo->dwClubID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行脚本
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_MB_QueryVideoInfo"), true);

    // 执行成功
    if (lResultCode == DB_SUCCESS) {
      // 变量定义
      CMD_MB_S_QueryVideoInfoResult VideoParameter;
      BYTE cbDataBuffer[SOCKET_TCP_PACKET - 1024] = {0};
      CMD_MB_S_QueryListCount* pVideoInfo = (CMD_MB_S_QueryListCount*)cbDataBuffer;
      uint8_t* pDataBuffer = cbDataBuffer + sizeof(CMD_MB_S_QueryListCount);

      // 设置变量
      WORD wVideoCount = 0;
      WORD wSendDataSize = sizeof(CMD_MB_S_QueryListCount);

      // 变量定义
      while (!m_PlatformDBModule->IsRecordsetEnd()) {
        // 读取数据
        ZeroMemory(&VideoParameter, sizeof(VideoParameter));
        VideoParameter.iVideoType = m_PlatformDBAide.GetValue_BYTE(TEXT("QueryType"));

        VideoParameter.dwUserID = m_PlatformDBAide.GetValue_DWORD(TEXT("UserID"));
        m_PlatformDBAide.GetValue_String(TEXT("NickName"), VideoParameter.szNickName, std::size(VideoParameter.szNickName));
        VideoParameter.dwGameID = m_PlatformDBAide.GetValue_DWORD(TEXT("GameID"));
        VideoParameter.wFaceID = m_PlatformDBAide.GetValue_WORD(TEXT("FaceID"));
        VideoParameter.dwCustomID = m_PlatformDBAide.GetValue_DWORD(TEXT("CustomID"));
        VideoParameter.lTotalScore = m_PlatformDBAide.GetValue_LONGLONG(TEXT("TotalScore"));
        m_PlatformDBAide.GetValue_SystemTime(TEXT("CreateTime"), VideoParameter.CreateTime);
        VideoParameter.wChairID = m_PlatformDBAide.GetValue_WORD(TEXT("ChairID"));
        VideoParameter.wKindID = m_PlatformDBAide.GetValue_WORD(TEXT("KindID"));
        VideoParameter.cbGender = m_PlatformDBAide.GetValue_BYTE(TEXT("Gender"));

        if (VideoParameter.iVideoType == 0) {
          VideoParameter.dwPersonalRoomID = m_PlatformDBAide.GetValue_DWORD(TEXT("RoomID"));
          m_PlatformDBAide.GetValue_String(TEXT("PersonalGUID"), VideoParameter.szPersonalGUID, std::size(VideoParameter.szPersonalGUID));
          VideoParameter.cbCreateRoom = m_PlatformDBAide.GetValue_BYTE(TEXT("CreateRoom"));
        } else {
          // 读取数据
          m_PlatformDBAide.GetValue_String(TEXT("VideoNumber"), VideoParameter.szVideoNumber, LEN_VIDEO_NUMBER);
        }
        if (pQueryVideo->dwPlayBack > 0) {
          VideoParameter.dwPlayBackUserID = m_PlatformDBAide.GetValue_DWORD(TEXT("PlayUserID"));
        }

        // 发送判断
        if (wSendDataSize + sizeof(CMD_MB_S_QueryVideoInfoResult) > sizeof(cbDataBuffer)) {
          pVideoInfo->wListCount = wVideoCount + 1;
          // 发送数据
          EmitDataBaseEngineResult(DBO_MB_VIDEO_LIST, dwContextID, pVideoInfo, wSendDataSize);

          // 重置变量
          wVideoCount = 0;
          wSendDataSize = sizeof(CMD_MB_S_QueryListCount);
          pDataBuffer = cbDataBuffer + sizeof(CMD_MB_S_QueryVideoInfoResult);
        }

        // 拷贝数据
        CopyMemory(pDataBuffer, &VideoParameter, sizeof(CMD_MB_S_QueryVideoInfoResult));

        // 设置变量
        wVideoCount++;
        wSendDataSize += sizeof(CMD_MB_S_QueryVideoInfoResult);
        pDataBuffer += sizeof(CMD_MB_S_QueryVideoInfoResult);

        // 移动记录
        m_PlatformDBModule->MoveToNext();
      }

      // 剩余发送
      if (wVideoCount > 0 && wSendDataSize > 0) {
        pVideoInfo->wListCount = wVideoCount;
        // 发送数据
        EmitDataBaseEngineResult(DBO_MB_VIDEO_LIST, dwContextID, pVideoInfo, wSendDataSize);
      }

      // 发送通知
      EmitDataBaseEngineResult(DBO_MB_VIDEO_LIST_END, dwContextID, nullptr, 0);
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
}

// 视频详情
bool CDataBaseEngineSink::OnRequestVideoDetails(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_MB_QueryVideoDetails));
    if (wDataSize != sizeof(DBR_MB_QueryVideoDetails))
      return false;

    // 请求处理
    DBR_MB_QueryVideoDetails* pQueryVideo = (DBR_MB_QueryVideoDetails*)pData;

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@szPersonalGUID"), pQueryVideo->szPersonalGUID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行脚本
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_MB_QueryVideoDetails"), true);

    // 执行成功
    if (lResultCode == DB_SUCCESS) {
      // 变量定义
      CMD_MB_S_QueryVideoDetailsResult VideoParameter;
      BYTE cbDataBuffer[SOCKET_TCP_PACKET - 1024] = {0};
      CMD_MB_S_QueryListCount* pVideoInfo = (CMD_MB_S_QueryListCount*)cbDataBuffer;
      uint8_t* pDataBuffer = cbDataBuffer + sizeof(CMD_MB_S_QueryListCount);

      // 设置变量
      WORD wVideoCount = 0;
      WORD wSendDataSize = sizeof(CMD_MB_S_QueryListCount);

      // 变量定义
      while (!m_PlatformDBModule->IsRecordsetEnd()) {
        // 初始化
        ZeroMemory(&VideoParameter, sizeof(VideoParameter));
        // 读取数据
        m_PlatformDBAide.GetValue_String(TEXT("VideoNumber"), VideoParameter.szVideoNumber, LEN_VIDEO_NUMBER);

        m_PlatformDBAide.GetValue_String(TEXT("NickName"), VideoParameter.szNickName, std::size(VideoParameter.szNickName));
        m_PlatformDBAide.GetValue_String(TEXT("PersonalGUID"), VideoParameter.szPersonalGUID, std::size(VideoParameter.szPersonalGUID));
        VideoParameter.dwUserID = m_PlatformDBAide.GetValue_DWORD(TEXT("UserID"));
        VideoParameter.lScore = m_PlatformDBAide.GetValue_LONGLONG(TEXT("Score"));
        VideoParameter.dwGamesNum = m_PlatformDBAide.GetValue_WORD(TEXT("GameNum"));
        VideoParameter.cbGameMode = m_PlatformDBAide.GetValue_BYTE(TEXT("GameMode"));
        VideoParameter.cbLoopCount = m_PlatformDBAide.GetValue_BYTE(TEXT("LoopCount"));

        // 发送判断
        if (wSendDataSize + sizeof(CMD_MB_S_QueryVideoDetailsResult) > sizeof(cbDataBuffer)) {
          pVideoInfo->wListCount = wVideoCount + 1;
          // 发送数据
          EmitDataBaseEngineResult(DBO_MB_VIDEO_DETAILS, dwContextID, pVideoInfo, wSendDataSize);

          // 重置变量
          wVideoCount = 0;
          wSendDataSize = sizeof(CMD_MB_S_QueryListCount);
          pDataBuffer = cbDataBuffer + sizeof(CMD_MB_S_QueryVideoDetailsResult);
        }

        // 拷贝数据
        CopyMemory(pDataBuffer, &VideoParameter, sizeof(CMD_MB_S_QueryVideoDetailsResult));

        // 设置变量
        wVideoCount++;
        wSendDataSize += sizeof(CMD_MB_S_QueryVideoDetailsResult);
        pDataBuffer += sizeof(CMD_MB_S_QueryVideoDetailsResult);

        // 移动记录
        m_PlatformDBModule->MoveToNext();
      }

      // 剩余发送
      if (wVideoCount > 0 && wSendDataSize > 0) {
        pVideoInfo->wListCount = wVideoCount;
        // 发送数据
        EmitDataBaseEngineResult(DBO_MB_VIDEO_DETAILS, dwContextID, pVideoInfo, wSendDataSize);
      }

      // 发送通知
      EmitDataBaseEngineResult(DBO_MB_VIDEO_DETAILS_END, dwContextID, nullptr, 0);
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
}

// 视频详情
bool CDataBaseEngineSink::OnRequestVideoDetailsByRoomID(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_MB_QueryVideoDetailsByRoomID));
    if (wDataSize != sizeof(DBR_MB_QueryVideoDetailsByRoomID))
      return false;

    // 请求处理
    DBR_MB_QueryVideoDetailsByRoomID* pQueryVideo = (DBR_MB_QueryVideoDetailsByRoomID*)pData;

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwPersonalRoomID"), pQueryVideo->dwPersonalRoomID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行脚本
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_MB_QueryVideoDetailsByRoomID"), true);

    // 执行成功
    if (lResultCode == DB_SUCCESS) {
      // 变量定义
      CMD_MB_S_QueryVideoDetailsResult VideoParameter;
      BYTE cbDataBuffer[SOCKET_TCP_PACKET - 1024] = {0};
      CMD_MB_S_QueryListCount* pVideoInfo = (CMD_MB_S_QueryListCount*)cbDataBuffer;
      uint8_t* pDataBuffer = cbDataBuffer + sizeof(CMD_MB_S_QueryListCount);

      // 设置变量
      WORD wVideoCount = 0;
      WORD wSendDataSize = sizeof(CMD_MB_S_QueryListCount);

      // 变量定义
      while (!m_PlatformDBModule->IsRecordsetEnd()) {
        // 读取数据
        m_PlatformDBAide.GetValue_String(TEXT("VideoNumber"), VideoParameter.szVideoNumber, LEN_VIDEO_NUMBER);

        m_PlatformDBAide.GetValue_String(TEXT("NickName"), VideoParameter.szNickName, std::size(VideoParameter.szNickName));
        m_PlatformDBAide.GetValue_String(TEXT("PersonalGUID"), VideoParameter.szPersonalGUID, std::size(VideoParameter.szPersonalGUID));
        VideoParameter.dwUserID = m_PlatformDBAide.GetValue_DWORD(TEXT("UserID"));
        VideoParameter.lScore = m_PlatformDBAide.GetValue_LONGLONG(TEXT("Score"));
        VideoParameter.dwGamesNum = m_PlatformDBAide.GetValue_WORD(TEXT("GameNum"));
        VideoParameter.cbGameMode = m_PlatformDBAide.GetValue_BYTE(TEXT("GameMode"));
        VideoParameter.cbLoopCount = m_PlatformDBAide.GetValue_BYTE(TEXT("LoopCount"));

        // 发送判断
        if (wSendDataSize + sizeof(CMD_MB_S_QueryVideoDetailsResult) > sizeof(cbDataBuffer)) {
          pVideoInfo->wListCount = wVideoCount + 1;
          // 发送数据
          EmitDataBaseEngineResult(DBO_MB_VIDEO_DETAILS, dwContextID, pVideoInfo, wSendDataSize);

          // 重置变量
          wVideoCount = 0;
          wSendDataSize = sizeof(CMD_MB_S_QueryListCount);
          pDataBuffer = cbDataBuffer + sizeof(CMD_MB_S_QueryVideoDetailsResult);
        }

        // 拷贝数据
        CopyMemory(pDataBuffer, &VideoParameter, sizeof(CMD_MB_S_QueryVideoDetailsResult));

        // 设置变量
        wVideoCount++;
        wSendDataSize += sizeof(CMD_MB_S_QueryVideoDetailsResult);
        pDataBuffer += sizeof(CMD_MB_S_QueryVideoDetailsResult);

        // 移动记录
        m_PlatformDBModule->MoveToNext();
      }

      // 剩余发送
      if (wVideoCount > 0 && wSendDataSize > 0) {
        pVideoInfo->wListCount = wVideoCount;
        // 发送数据
        EmitDataBaseEngineResult(DBO_MB_VIDEO_DETAILS, dwContextID, pVideoInfo, wSendDataSize);
      }

      // 发送通知
      EmitDataBaseEngineResult(DBO_MB_VIDEO_DETAILS_END, dwContextID, nullptr, 0);
    }

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
}
// 查询回放码
bool CDataBaseEngineSink::OnRequestPlayBackCodeYZ(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_MB_QueryPlayBackCodeYZ));
    if (wDataSize != sizeof(DBR_MB_QueryPlayBackCodeYZ))
      return false;

    // 请求处理
    DBR_MB_QueryPlayBackCodeYZ* pQueryCode = (DBR_MB_QueryPlayBackCodeYZ*)pData;

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@szUserID"), pQueryCode->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@szPersonalGUID"), pQueryCode->szPersonalGUID);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行脚本
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_MB_QueryPlayBackCodeYZ"), true);

    // 构造结构
    DBR_MB_QueryPlayBackCode_YZ_Result PlatformParameter;
    ZeroMemory(&PlatformParameter, sizeof(PlatformParameter));
    // 执行成功
    if (lResultCode == DB_SUCCESS) {
      // 记录判断
      if (!m_PlatformDBModule->IsRecordsetEnd()) {
        // 读取数据
        PlatformParameter.dwPlayBackCode = m_PlatformDBAide.GetValue_DWORD(TEXT("Code"));
        PlatformParameter.dwUserID = pQueryCode->dwUserID;
        LSTRCPYN(PlatformParameter.szPersonalGUID, pQueryCode->szPersonalGUID, std::size(PlatformParameter.szPersonalGUID));
      }
    }
    // 发送数据
    EmitDataBaseEngineResult(DBO_MB_PLAY_BACK_CODE_YZ_RESULT, dwContextID, &PlatformParameter, sizeof(PlatformParameter));

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
}

// 查询回放码
bool CDataBaseEngineSink::OnRequestPlayBackCode(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 效验参数
    ASSERT(wDataSize == sizeof(DBR_MB_QueryPlayBackCode));
    if (wDataSize != sizeof(DBR_MB_QueryPlayBackCode))
      return false;

    // 请求处理
    DBR_MB_QueryPlayBackCode* pQueryCode = (DBR_MB_QueryPlayBackCode*)pData;

    // 构造参数
    m_TreasureDBAide.ResetParameter();
    m_TreasureDBAide.AddParameter(TEXT("@szUserID"), pQueryCode->dwUserID);
    m_TreasureDBAide.AddParameter(TEXT("@szVideoNumber"), pQueryCode->szVideoNumber);

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_TreasureDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行脚本
    LONG lResultCode = m_TreasureDBAide.ExecuteProcess(TEXT("GSP_MB_QueryPlayBackCode"), true);

    // 构造结构
    DBR_MB_QueryPlayBackCode_Result PlatformParameter;
    ZeroMemory(&PlatformParameter, sizeof(PlatformParameter));
    // 执行成功
    if (lResultCode == DB_SUCCESS) {
      // 记录判断
      if (!m_TreasureDBModule->IsRecordsetEnd()) {
        // 读取数据
        PlatformParameter.dwPlayBackCode = m_TreasureDBAide.GetValue_DWORD(TEXT("Code"));
        PlatformParameter.dwUserID = pQueryCode->dwUserID;

        StrnCpyT(PlatformParameter.szVideoNumber, pQueryCode->szVideoNumber, std::size(PlatformParameter.szVideoNumber));
      }
    }
    // 发送数据
    EmitDataBaseEngineResult(DBO_MB_PLAY_BACK_CODE_RESULT, dwContextID, &PlatformParameter, sizeof(PlatformParameter));

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
}

// 加载列表
bool CDataBaseEngineSink::OnRequestLoadGameList(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 变量定义
    WORD wPacketSize = 0;
    BYTE cbBuffer[MAX_ASYNCHRONISM_DATA];

    // 加载类型
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_LoadGameTypeItem"), true);

    // 发送种类
    wPacketSize = 0;
    DBO_GP_GameType* pGameType = nullptr;
    while (!m_PlatformDBModule->IsRecordsetEnd()) {
      // 发送信息
      if ((wPacketSize + sizeof(DBO_GP_GameType)) > (sizeof(cbBuffer) - sizeof(NTY_DataBaseEvent))) {
        EmitDataBaseEngineResult(DBO_GP_GAME_TYPE_ITEM, dwContextID, cbBuffer, wPacketSize);
        wPacketSize = 0;
      }

      // 读取信息
      pGameType = (DBO_GP_GameType*)(cbBuffer + wPacketSize);
      pGameType->wSortID = m_PlatformDBAide.GetValue_WORD(TEXT("SortID"));
      pGameType->wJoinID = m_PlatformDBAide.GetValue_WORD(TEXT("JoinID"));
      pGameType->wTypeID = m_PlatformDBAide.GetValue_WORD(TEXT("TypeID"));
      m_PlatformDBAide.GetValue_String(TEXT("TypeName"), pGameType->szTypeName, std::size(pGameType->szTypeName));

      // 设置位移
      wPacketSize += sizeof(DBO_GP_GameType);

      // 移动记录
      m_PlatformDBModule->MoveToNext();
    }
    if (wPacketSize > 0)
      EmitDataBaseEngineResult(DBO_GP_GAME_TYPE_ITEM, dwContextID, cbBuffer, wPacketSize);

    // 读取类型
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_LoadGameKindItem"), true);

    // 发送类型
    wPacketSize = 0;
    DBO_GP_GameKind* pGameKind = nullptr;
    while (!m_PlatformDBModule->IsRecordsetEnd()) {
      // 发送信息
      if ((wPacketSize + sizeof(DBO_GP_GameKind)) > (sizeof(cbBuffer) - sizeof(NTY_DataBaseEvent))) {
        EmitDataBaseEngineResult(DBO_GP_GAME_KIND_ITEM, dwContextID, cbBuffer, wPacketSize);
        wPacketSize = 0;
      }

      // 读取信息
      pGameKind = (DBO_GP_GameKind*)(cbBuffer + wPacketSize);
      pGameKind->wSortID = m_PlatformDBAide.GetValue_WORD(TEXT("SortID"));
      pGameKind->wTypeID = m_PlatformDBAide.GetValue_WORD(TEXT("TypeID"));
      pGameKind->wJoinID = m_PlatformDBAide.GetValue_WORD(TEXT("JoinID"));
      pGameKind->wKindID = m_PlatformDBAide.GetValue_WORD(TEXT("KindID"));
      pGameKind->wGameID = m_PlatformDBAide.GetValue_WORD(TEXT("GameID"));
      m_PlatformDBAide.GetValue_String(TEXT("KindName"), pGameKind->szKindName, std::size(pGameKind->szKindName));
      m_PlatformDBAide.GetValue_String(TEXT("ProcessName"), pGameKind->szProcessName, std::size(pGameKind->szProcessName));
      pGameKind->wRecommend = m_PlatformDBAide.GetValue_WORD(TEXT("Recommend"));
      pGameKind->wGameFlag = m_PlatformDBAide.GetValue_WORD(TEXT("GameFlag"));

      // 设置位移
      wPacketSize += sizeof(DBO_GP_GameKind);

      // 移动记录
      m_PlatformDBModule->MoveToNext();
    }
    if (wPacketSize > 0)
      EmitDataBaseEngineResult(DBO_GP_GAME_KIND_ITEM, dwContextID, cbBuffer, wPacketSize);

    // 读取节点
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_LoadGameNodeItem"), true);

    // 发送节点
    wPacketSize = 0;
    DBO_GP_GameNode* pGameNode = nullptr;
    while (!m_PlatformDBModule->IsRecordsetEnd()) {
      // 发送信息
      if ((wPacketSize + sizeof(DBO_GP_GameNode)) > (sizeof(cbBuffer) - sizeof(NTY_DataBaseEvent))) {
        EmitDataBaseEngineResult(DBO_GP_GAME_NODE_ITEM, dwContextID, cbBuffer, wPacketSize);
        wPacketSize = 0;
      }

      // 读取信息
      pGameNode = (DBO_GP_GameNode*)(cbBuffer + wPacketSize);
      pGameNode->wSortID = m_PlatformDBAide.GetValue_WORD(TEXT("SortID"));
      pGameNode->wKindID = m_PlatformDBAide.GetValue_WORD(TEXT("KindID"));
      pGameNode->wJoinID = m_PlatformDBAide.GetValue_WORD(TEXT("JoinID"));
      pGameNode->wNodeID = m_PlatformDBAide.GetValue_WORD(TEXT("NodeID"));
      m_PlatformDBAide.GetValue_String(TEXT("NodeName"), pGameNode->szNodeName, std::size(pGameNode->szNodeName));

      // 设置位移
      wPacketSize += sizeof(DBO_GP_GameNode);

      // 移动记录
      m_PlatformDBModule->MoveToNext();
    }
    if (wPacketSize > 0)
      EmitDataBaseEngineResult(DBO_GP_GAME_NODE_ITEM, dwContextID, cbBuffer, wPacketSize);

    // 读取定制
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_LoadGamePageItem"), true);

    // 发送定制
    wPacketSize = 0;
    DBO_GP_GamePage* pGamePage = nullptr;
    while (!m_PlatformDBModule->IsRecordsetEnd()) {
      // 发送信息
      if ((wPacketSize + sizeof(DBO_GP_GamePage)) > (sizeof(cbBuffer) - sizeof(NTY_DataBaseEvent))) {
        EmitDataBaseEngineResult(DBO_GP_GAME_PAGE_ITEM, dwContextID, cbBuffer, wPacketSize);
        wPacketSize = 0;
      }

      // 读取信息
      pGamePage = (DBO_GP_GamePage*)(cbBuffer + wPacketSize);
      pGamePage->wKindID = m_PlatformDBAide.GetValue_WORD(TEXT("KindID"));
      pGamePage->wNodeID = m_PlatformDBAide.GetValue_WORD(TEXT("NodeID"));
      pGamePage->wSortID = m_PlatformDBAide.GetValue_WORD(TEXT("SortID"));
      pGamePage->wPageID = m_PlatformDBAide.GetValue_WORD(TEXT("PageID"));
      pGamePage->wOperateType = m_PlatformDBAide.GetValue_WORD(TEXT("OperateType"));
      m_PlatformDBAide.GetValue_String(TEXT("DisplayName"), pGamePage->szDisplayName, std::size(pGamePage->szDisplayName));

      // 设置位移
      wPacketSize += sizeof(DBO_GP_GamePage);

      // 移动记录
      m_PlatformDBModule->MoveToNext();
    }
    if (wPacketSize > 0)
      EmitDataBaseEngineResult(DBO_GP_GAME_PAGE_ITEM, dwContextID, cbBuffer, wPacketSize);

    // 变量定义
    DBO_GP_GameListResult GameListResult;
    ZeroMemory(&GameListResult, sizeof(GameListResult));

    // 设置变量
    GameListResult.cbSuccess = TRUE;

    // 发送消息
    EmitDataBaseEngineResult(DBO_GP_GAME_LIST_RESULT, dwContextID, &GameListResult, sizeof(GameListResult));

    return true;
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);

    // 变量定义
    DBO_GP_GameListResult GameListResult;
    ZeroMemory(&GameListResult, sizeof(GameListResult));

    // 设置变量
    GameListResult.cbSuccess = FALSE;

    // 发送消息
    EmitDataBaseEngineResult(DBO_GP_GAME_LIST_RESULT, dwContextID, &GameListResult, sizeof(GameListResult));

    return false;
  }

  return true;
}

// 在线信息
bool CDataBaseEngineSink::OnRequestOnLineCountInfo(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 变量定义
    DBR_GP_OnLineCountInfo* pOnLineCountInfo = (DBR_GP_OnLineCountInfo*)pData;
    WORD wHeadSize = (sizeof(DBR_GP_OnLineCountInfo) - sizeof(pOnLineCountInfo->OnLineCountKind));

    // 效验数据
    ASSERT((wDataSize >= wHeadSize) && (wDataSize == (wHeadSize + pOnLineCountInfo->wKindCount * sizeof(tagOnLineInfoKindEx))));
    if ((wDataSize < wHeadSize) || (wDataSize != (wHeadSize + pOnLineCountInfo->wKindCount * sizeof(tagOnLineInfoKindEx))))
      return false;

    // 机器标识
    TCHAR szMachineID[LEN_MACHINE_ID];
    CWHService::GetMachineID(szMachineID);

    // 构造信息
    TCHAR szOnLineCountKind[2048] = TEXT("");
    for (WORD i = 0; i < pOnLineCountInfo->wKindCount; i++) {
      INT nLength = StrLenT(szOnLineCountKind);
      SnprintfT(&szOnLineCountKind[nLength], std::size(szOnLineCountKind) - nLength, /*std::size(szOnLineCountKind) - nLength,*/
                TEXT("%d:%ld;"), pOnLineCountInfo->OnLineCountKind[i].wKindID, pOnLineCountInfo->OnLineCountKind[i].dwOnLineCount);
    }

    // 机器信息
    TCHAR szAndroidCountKind[2048] = TEXT("");
    for (WORD j = 0; j < pOnLineCountInfo->wKindCount; j++) {
      INT nLength = StrLenT(szAndroidCountKind);
      SnprintfT(&szAndroidCountKind[nLength], std::size(szAndroidCountKind) - nLength, /*std::size(szOnLineCountKind) - nLength,*/
                TEXT("%d:%ld;"), pOnLineCountInfo->OnLineCountKind[j].wKindID, pOnLineCountInfo->OnLineCountKind[j].dwAndroidCount);
    }

    // 构造参数
    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@strMachineID"), szMachineID);
    m_PlatformDBAide.AddParameter(TEXT("@strMachineServer"), init_parameter_->server_name_);
    m_PlatformDBAide.AddParameter(TEXT("@dwOnLineCountSum"), pOnLineCountInfo->dwOnLineCountSum);
    m_PlatformDBAide.AddParameter(TEXT("@dwAndroidCountSum"), pOnLineCountInfo->dwAndroidCountSum);
    m_PlatformDBAide.AddParameter(TEXT("@strOnLineCountKind"), szOnLineCountKind);
    m_PlatformDBAide.AddParameter(TEXT("@strAndroidCountKind"), szAndroidCountKind);

    // 执行命令
    m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GP_OnLineCountInfo"), false);

    return true;
  } catch (IDataBaseException* pIException) {
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }

  return true;
}

// 平台配置
bool CDataBaseEngineSink::OnRequestPlatformParameter(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 构造参数
    m_PlatformDBAide.ResetParameter();

    // 执行命令
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GR_LoadPlatformParameter"), true);

    // 构造结构
    DBO_GP_PlatformParameter PlatformParameter;
    ZeroMemory(&PlatformParameter, sizeof(PlatformParameter));

    // 执行成功
    if (lResultCode == DB_SUCCESS) {
      // 记录判断
      if (!m_PlatformDBModule->IsRecordsetEnd()) {
        // 读取数据
        PlatformParameter.dwExchangeRate = m_PlatformDBAide.GetValue_DWORD(TEXT("ExchangeRate"));
        PlatformParameter.dwPresentExchangeRate = m_PlatformDBAide.GetValue_DWORD(TEXT("PresentExchangeRate"));
        //	PlatformParameter.dwRateGold = m_PlatformDBAide.GetValue_DWORD(TEXT("RateGold"));
      }
    }

    // 发送数据
    EmitDataBaseEngineResult(DBO_GP_PLATFORM_PARAMETER, dwContextID, &PlatformParameter, sizeof(PlatformParameter));

    return true;
  } catch (IDataBaseException* pIException) {
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }

  return true;
}

// 加载任务
bool CDataBaseEngineSink::OnRequestLoadTaskList(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    // 构造参数
    m_PlatformDBAide.ResetParameter();

    // 执行命令
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GR_LoadTaskList"), true);

    // 执行成功
    if (lResultCode == DB_SUCCESS) {
      // 变量定义
      tagTaskParameter TaskParameter;
      BYTE cbDataBuffer[SOCKET_TCP_PACKET - 1024] = {0};
      DBO_GP_TaskListInfo* pTaskListInfo = (DBO_GP_TaskListInfo*)cbDataBuffer;
      uint8_t* pDataBuffer = cbDataBuffer + sizeof(DBO_GP_TaskListInfo);

      // 设置变量
      WORD wTaskCount = 0;
      WORD wSendDataSize = sizeof(DBO_GP_TaskListInfo);

      // 变量定义
      while (!m_PlatformDBModule->IsRecordsetEnd()) {
        // 读取数据
        TaskParameter.wTaskID = m_PlatformDBAide.GetValue_WORD(TEXT("TaskID"));
        TaskParameter.wTaskType = m_PlatformDBAide.GetValue_WORD(TEXT("TaskType"));
        TaskParameter.cbPlayerType = m_PlatformDBAide.GetValue_BYTE(TEXT("UserType"));
        TaskParameter.wKindID = m_PlatformDBAide.GetValue_WORD(TEXT("KindID"));
        TaskParameter.wTaskObject = m_PlatformDBAide.GetValue_WORD(TEXT("Innings"));
        TaskParameter.dwTimeLimit = m_PlatformDBAide.GetValue_DWORD(TEXT("TimeLimit"));
        TaskParameter.lStandardAwardGold = m_PlatformDBAide.GetValue_LONGLONG(TEXT("StandardAwardGold"));
        TaskParameter.lStandardAwardMedal = m_PlatformDBAide.GetValue_LONGLONG(TEXT("StandardAwardMedal"));
        TaskParameter.lMemberAwardGold = m_PlatformDBAide.GetValue_LONGLONG(TEXT("MemberAwardGold"));
        TaskParameter.lMemberAwardMedal = m_PlatformDBAide.GetValue_LONGLONG(TEXT("MemberAwardMedal"));

        // 描述信息
        m_PlatformDBAide.GetValue_String(TEXT("TaskName"), TaskParameter.szTaskName, std::size(TaskParameter.szTaskName));
        m_PlatformDBAide.GetValue_String(TEXT("TaskDescription"), TaskParameter.szTaskDescribe, std::size(TaskParameter.szTaskDescribe));

        // 发送判断
        if (wSendDataSize + sizeof(tagTaskParameter) > sizeof(cbDataBuffer)) {
          // 设置变量
          pTaskListInfo->wTaskCount = wTaskCount;

          // 发送数据
          EmitDataBaseEngineResult(DBO_GP_TASK_LIST, dwContextID, pTaskListInfo, wSendDataSize);

          // 重置变量
          wTaskCount = 0;
          wSendDataSize = sizeof(DBO_GP_TaskListInfo);
          pDataBuffer = cbDataBuffer + sizeof(DBO_GP_TaskListInfo);
        }

        // 拷贝数据
        CopyMemory(pDataBuffer, &TaskParameter, sizeof(tagTaskParameter));

        // 设置变量
        wTaskCount++;
        wSendDataSize += sizeof(tagTaskParameter);
        pDataBuffer += sizeof(tagTaskParameter);

        // 移动记录
        m_PlatformDBModule->MoveToNext();
      }

      // 剩余发送
      if (wTaskCount > 0 && wSendDataSize > 0) {
        // 设置变量
        pTaskListInfo->wTaskCount = wTaskCount;

        // 发送数据
        EmitDataBaseEngineResult(DBO_GP_TASK_LIST, dwContextID, pTaskListInfo, wSendDataSize);
      }

      // 发送通知
      EmitDataBaseEngineResult(DBO_GP_TASK_LIST_END, dwContextID, nullptr, 0);
    }

    return true;
  } catch (IDataBaseException* pIException) {
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }

  return true;
}

// 登录成功
VOID CDataBaseEngineSink::OnLogonDisposeResult(DWORD dwContextID, DWORD dwErrorCode, LPCTSTR pszErrorString, DWORD dwCheckUserRight,
                                               bool bMobileClient) {
  if (!bMobileClient) {
    if (dwErrorCode == DB_SUCCESS) {
      // 变量定义
      DBO_GP_LogonSuccess LogonSuccess;
      ZeroMemory(&LogonSuccess, sizeof(LogonSuccess));

      // 属性资料
      LogonSuccess.wFaceID = m_AccountsDBAide.GetValue_WORD(TEXT("FaceID"));
      LogonSuccess.dwUserID = m_AccountsDBAide.GetValue_DWORD(TEXT("UserID"));
      LogonSuccess.dwGameID = m_AccountsDBAide.GetValue_DWORD(TEXT("GameID"));
      LogonSuccess.dwCustomID = m_AccountsDBAide.GetValue_DWORD(TEXT("CustomID"));
      // LogonSuccess.dwExperience=m_AccountsDBAide.GetValue_DWORD(TEXT("Experience"));
      // LogonSuccess.lLoveLiness=m_AccountsDBAide.GetValue_LONGLONG(TEXT("LoveLiness"));
      m_AccountsDBAide.GetValue_String(TEXT("Accounts"), LogonSuccess.szAccounts, std::size(LogonSuccess.szAccounts));
      m_AccountsDBAide.GetValue_String(TEXT("NickName"), LogonSuccess.szNickName, std::size(LogonSuccess.szNickName));
      m_AccountsDBAide.GetValue_String(TEXT("DynamicPass"), LogonSuccess.szDynamicPass, std::size(LogonSuccess.szDynamicPass));

      // 用户成绩
      LogonSuccess.lUserScore = m_AccountsDBAide.GetValue_LONGLONG(TEXT("Score"));
      // LogonSuccess.lUserIngot=m_AccountsDBAide.GetValue_LONGLONG(TEXT("Ingot"));
      LogonSuccess.lUserInsure = m_AccountsDBAide.GetValue_LONGLONG(TEXT("Insure"));
      // LogonSuccess.dUserBeans= m_AccountsDBAide.GetValue_DOUBLE(TEXT("Beans"));

      // 用户资料
      LogonSuccess.cbGender = m_AccountsDBAide.GetValue_BYTE(TEXT("Gender"));
      LogonSuccess.cbMoorMachine = m_AccountsDBAide.GetValue_BYTE(TEXT("MoorMachine"));
      m_AccountsDBAide.GetValue_String(TEXT("UnderWrite"), LogonSuccess.szUnderWrite, std::size(LogonSuccess.szUnderWrite));

      // 会员资料
      LogonSuccess.cbMemberOrder = m_AccountsDBAide.GetValue_BYTE(TEXT("MemberOrder"));
      m_AccountsDBAide.GetValue_SystemTime(TEXT("MemberOverDate"), LogonSuccess.MemberOverDate);

      // 扩展信息
      LogonSuccess.cbInsureEnabled = m_AccountsDBAide.GetValue_BYTE(TEXT("InsureEnabled"));
      LogonSuccess.cbIsAgent = m_AccountsDBAide.GetValue_BYTE(TEXT("IsAgent"));
      LogonSuccess.dwCheckUserRight = dwCheckUserRight;

      // 获取信息
      LSTRCPYN(LogonSuccess.szDescribeString, pszErrorString, std::size(LogonSuccess.szDescribeString));

      //////////////////////////////////////////////////////////////////////////
      try {
        // 变量定义
        WORD wPacketSize = 0;
        BYTE cbBuffer[SOCKET_TCP_PACKET];

        // 读取类型
        m_AccountsDBAide.ResetParameter();
        m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), LogonSuccess.dwUserID);
        m_AccountsDBAide.AddParameter(TEXT("@dwDeviceID"), 1);
        // 输出参数
        TCHAR szDescribeString[128] = TEXT("");
        m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);
        m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_LoadAgentGameKindItem"), true);

        // 发送类型
        wPacketSize = 0;
        tagAgentGameKind* pGameKind = nullptr;
        while (!m_AccountsDBModule->IsRecordsetEnd()) {
          // 读取信息
          pGameKind = (tagAgentGameKind*)(cbBuffer + wPacketSize);
          pGameKind->wKindID = m_AccountsDBAide.GetValue_WORD(TEXT("KindID"));
          pGameKind->wSortID = m_AccountsDBAide.GetValue_WORD(TEXT("SortID"));

          // 设置位移
          wPacketSize += sizeof(tagAgentGameKind);

          // 移动记录
          m_AccountsDBModule->MoveToNext();
        }
        if (wPacketSize > 0)
          EmitDataBaseEngineResult(DBO_GP_AGENT_GAME_KIND_ITEM, dwContextID, cbBuffer, wPacketSize);
      } catch (...) {
        CLogger::Error(TEXT("{} → GSP_GP_LoadAgentGameKindItem 出现异常"), TEXT(__FUNCTION__));
      }
      //////////////////////////////////////////////////////////////////////////

      // 发送结果
      WORD wDataSize = CountStringBuffer(LogonSuccess.szDescribeString);
      WORD wHeadSize = sizeof(LogonSuccess) - sizeof(LogonSuccess.szDescribeString);
      EmitDataBaseEngineResult(DBO_GP_LOGON_SUCCESS, dwContextID, &LogonSuccess, wHeadSize + wDataSize);
    } else if (dwErrorCode == DB_NEEDMB) {
      // 登录成功
      DBO_GP_ValidateMBCard ValidateMBCard;
      ZeroMemory(&ValidateMBCard, sizeof(ValidateMBCard));

      // 读取变量
      // ValidateMBCard.uMBCardID=m_AccountsDBAide.GetValue_UINT(TEXT("PasswordID"));

      // 投递结果
      EmitDataBaseEngineResult(DBO_GP_VALIDATE_MBCARD, dwContextID, &ValidateMBCard, sizeof(ValidateMBCard));
    } else if (dwErrorCode == DB_PASSPORT) {
      // 投递结果
      EmitDataBaseEngineResult(DBO_GP_VALIDATE_PASSPORT, dwContextID, nullptr, 0);
    } else {
      // 变量定义
      DBO_GP_LogonFailure LogonFailure;
      ZeroMemory(&LogonFailure, sizeof(LogonFailure));

      // 构造数据
      LogonFailure.lResultCode = dwErrorCode;
      LSTRCPYN(LogonFailure.szDescribeString, pszErrorString, std::size(LogonFailure.szDescribeString));

      // 发送结果
      WORD wDataSize = CountStringBuffer(LogonFailure.szDescribeString);
      WORD wHeadSize = sizeof(LogonFailure) - sizeof(LogonFailure.szDescribeString);
      EmitDataBaseEngineResult(DBO_GP_LOGON_FAILURE, dwContextID, &LogonFailure, wHeadSize + wDataSize);
    }
  } else {
    if (dwErrorCode == DB_SUCCESS) {
      // 变量定义
      DBO_MB_LogonSuccess LogonSuccess;
      ZeroMemory(&LogonSuccess, sizeof(LogonSuccess));

      // 属性资料
      LogonSuccess.wFaceID = m_AccountsDBAide.GetValue_WORD(TEXT("FaceID"));
      LogonSuccess.cbGender = m_AccountsDBAide.GetValue_BYTE(TEXT("Gender"));
      LogonSuccess.dwCustomID = m_AccountsDBAide.GetValue_DWORD(TEXT("CustomID"));
      LogonSuccess.dwUserID = m_AccountsDBAide.GetValue_DWORD(TEXT("UserID"));
      LogonSuccess.dwGameID = m_AccountsDBAide.GetValue_DWORD(TEXT("GameID"));
      // LogonSuccess.dwExperience=m_AccountsDBAide.GetValue_DWORD(TEXT("Experience"));
      // LogonSuccess.lLoveLiness=m_AccountsDBAide.GetValue_LONGLONG(TEXT("LoveLiness"));
      m_AccountsDBAide.GetValue_String(TEXT("Accounts"), LogonSuccess.szAccounts, std::size(LogonSuccess.szAccounts));
      m_AccountsDBAide.GetValue_String(TEXT("NickName"), LogonSuccess.szNickName, std::size(LogonSuccess.szNickName));
      m_AccountsDBAide.GetValue_String(TEXT("DynamicPass"), LogonSuccess.szDynamicPass, std::size(LogonSuccess.szDynamicPass));
      m_AccountsDBAide.GetValue_String(TEXT("UnderWrite"), LogonSuccess.szUnderWrite, std::size(LogonSuccess.szUnderWrite));

      // 用户成绩
      LogonSuccess.lUserScore = m_AccountsDBAide.GetValue_LONGLONG(TEXT("Score"));
      // LogonSuccess.lUserIngot=m_AccountsDBAide.GetValue_LONGLONG(TEXT("Ingot"));
      LogonSuccess.lUserInsure = m_AccountsDBAide.GetValue_LONGLONG(TEXT("Insure"));
      // LogonSuccess.dUserBeans= m_AccountsDBAide.GetValue_DOUBLE(TEXT("Beans"));
      LogonSuccess.lDiamond = m_AccountsDBAide.GetValue_LONGLONG(TEXT("Diamond"));

      // 会员资料
      LogonSuccess.cbMemberOrder = m_AccountsDBAide.GetValue_BYTE(TEXT("MemberOrder"));
      m_AccountsDBAide.GetValue_SystemTime(TEXT("MemberOverDate"), LogonSuccess.MemberOverDate);

      // 扩展信息
      LogonSuccess.cbInsureEnabled = m_AccountsDBAide.GetValue_BYTE(TEXT("InsureEnabled"));
      LogonSuccess.cbIsAgent = m_AccountsDBAide.GetValue_BYTE(TEXT("IsAgent"));
      LogonSuccess.cbMoorMachine = m_AccountsDBAide.GetValue_BYTE(TEXT("MoorMachine"));

      // 房卡信息，房卡锁表
      // LogonSuccess.lRoomCard=m_AccountsDBAide.GetValue_LONGLONG(TEXT("RoomCard"));
      LogonSuccess.dwLockServerID = m_AccountsDBAide.GetValue_DWORD(TEXT("LockServerID"));
      LogonSuccess.dwKindID = m_AccountsDBAide.GetValue_DWORD(TEXT("KindID"));

      // 获取信息
      LSTRCPYN(LogonSuccess.szDescribeString, pszErrorString, std::size(LogonSuccess.szDescribeString));

      //////////////////////////////////////////////////////////////////////////
      try {
        // 变量定义
        WORD wPacketSize = 0;
        BYTE cbBuffer[SOCKET_TCP_PACKET];

        // 读取类型
        m_AccountsDBAide.ResetParameter();
        m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), LogonSuccess.dwUserID);
        m_AccountsDBAide.AddParameter(TEXT("@dwDeviceID"), 2);
        // 输出参数
        TCHAR szDescribeString[128] = TEXT("");
        m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);
        m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_LoadAgentGameKindItem"), true);

        // 发送类型
        wPacketSize = 0;
        tagAgentGameKind* pGameKind = nullptr;
        while (!m_AccountsDBModule->IsRecordsetEnd()) {
          // 读取信息
          pGameKind = (tagAgentGameKind*)(cbBuffer + wPacketSize);
          pGameKind->wKindID = m_AccountsDBAide.GetValue_WORD(TEXT("KindID"));
          pGameKind->wSortID = m_AccountsDBAide.GetValue_WORD(TEXT("SortID"));

          // 设置位移
          wPacketSize += sizeof(tagAgentGameKind);

          // 移动记录
          m_AccountsDBModule->MoveToNext();
        }
        if (wPacketSize > 0)
          EmitDataBaseEngineResult(DBO_MB_AGENT_GAME_KIND_ITEM, dwContextID, cbBuffer, wPacketSize);
      } catch (...) {
        CLogger::Error(TEXT("{} → GSP_GP_LoadAgentGameKindItem 出现异常"), TEXT(__FUNCTION__));
      }
      //////////////////////////////////////////////////////////////////////////

      // 发送结果
      WORD wDataSize = CountStringBuffer(LogonSuccess.szDescribeString);
      WORD wHeadSize = sizeof(LogonSuccess) - sizeof(LogonSuccess.szDescribeString);
      EmitDataBaseEngineResult(DBO_MB_LOGON_SUCCESS, dwContextID, &LogonSuccess, wHeadSize + wDataSize);
    } else {
      // 变量定义
      DBO_MB_LogonFailure LogonFailure;
      ZeroMemory(&LogonFailure, sizeof(LogonFailure));

      // 构造数据
      LogonFailure.lResultCode = dwErrorCode;
      LSTRCPYN(LogonFailure.szDescribeString, pszErrorString, std::size(LogonFailure.szDescribeString));

      // 发送结果
      WORD wDataSize = CountStringBuffer(LogonFailure.szDescribeString);
      WORD wHeadSize = sizeof(LogonFailure) - sizeof(LogonFailure.szDescribeString);
      EmitDataBaseEngineResult(DBO_MB_LOGON_FAILURE, dwContextID, &LogonFailure, wHeadSize + wDataSize);
    }
  }
}

// 银行结果
VOID CDataBaseEngineSink::OnInsureDisposeResult(DWORD dwContextID, DWORD dwErrorCode, LPCTSTR pszErrorString, bool bMobileClient) {
  if (dwErrorCode == DB_SUCCESS) {
    // 变量定义
    DBO_GP_UserInsureSuccess UserInsureSuccess;
    ZeroMemory(&UserInsureSuccess, sizeof(UserInsureSuccess));

    // 构造变量
    UserInsureSuccess.dwUserID = m_TreasureDBAide.GetValue_DWORD(TEXT("UserID"));
    UserInsureSuccess.lSourceScore = m_TreasureDBAide.GetValue_LONGLONG(TEXT("SourceScore"));
    UserInsureSuccess.lSourceInsure = m_TreasureDBAide.GetValue_LONGLONG(TEXT("SourceInsure"));
    UserInsureSuccess.lInsureRevenue = m_TreasureDBAide.GetValue_LONGLONG(TEXT("InsureRevenue"));
    UserInsureSuccess.lVariationScore = m_TreasureDBAide.GetValue_LONGLONG(TEXT("VariationScore"));
    UserInsureSuccess.lVariationInsure = m_TreasureDBAide.GetValue_LONGLONG(TEXT("VariationInsure"));
    LSTRCPYN(UserInsureSuccess.szDescribeString, pszErrorString, std::size(UserInsureSuccess.szDescribeString));

    // 发送结果
    WORD wDataSize = CountStringBuffer(UserInsureSuccess.szDescribeString);
    WORD wHeadSize = sizeof(UserInsureSuccess) - sizeof(UserInsureSuccess.szDescribeString);
    EmitDataBaseEngineResult(DBO_GP_USER_INSURE_SUCCESS, dwContextID, &UserInsureSuccess, wHeadSize + wDataSize);
  } else {
    // 变量定义
    DBO_GP_UserInsureFailure UserInsureFailure;
    ZeroMemory(&UserInsureFailure, sizeof(UserInsureFailure));

    // 构造变量
    UserInsureFailure.lResultCode = dwErrorCode;
    LSTRCPYN(UserInsureFailure.szDescribeString, pszErrorString, std::size(UserInsureFailure.szDescribeString));

    // 发送结果
    WORD wDataSize = CountStringBuffer(UserInsureFailure.szDescribeString);
    WORD wHeadSize = sizeof(UserInsureFailure) - sizeof(UserInsureFailure.szDescribeString);
    EmitDataBaseEngineResult(DBO_GP_USER_INSURE_FAILURE, dwContextID, &UserInsureFailure, wHeadSize + wDataSize);
  }
}

// 操作结果
VOID CDataBaseEngineSink::OnOperateDisposeResult(DWORD dwContextID, DWORD dwErrorCode, LPCTSTR pszErrorString, bool bMobileClient,
                                                 bool bCloseSocket) {
  if (dwErrorCode == DB_SUCCESS) {
    // 变量定义
    DBO_GP_OperateSuccess OperateSuccess;
    ZeroMemory(&OperateSuccess, sizeof(OperateSuccess));

    // 构造变量
    OperateSuccess.bCloseSocket = bCloseSocket;
    OperateSuccess.lResultCode = dwErrorCode;
    LSTRCPYN(OperateSuccess.szDescribeString, pszErrorString, std::size(OperateSuccess.szDescribeString));

    // 发送结果
    WORD wDataSize = CountStringBuffer(OperateSuccess.szDescribeString);
    WORD wHeadSize = sizeof(OperateSuccess) - sizeof(OperateSuccess.szDescribeString);
    EmitDataBaseEngineResult(DBO_GP_OPERATE_SUCCESS, dwContextID, &OperateSuccess, wHeadSize + wDataSize);
  } else {
    // 变量定义
    DBO_GP_OperateFailure OperateFailure;
    ZeroMemory(&OperateFailure, sizeof(OperateFailure));

    // 构造变量
    OperateFailure.bCloseSocket = bCloseSocket;
    OperateFailure.lResultCode = dwErrorCode;
    LSTRCPYN(OperateFailure.szDescribeString, pszErrorString, std::size(OperateFailure.szDescribeString));

    // 发送结果
    WORD wDataSize = CountStringBuffer(OperateFailure.szDescribeString);
    WORD wHeadSize = sizeof(OperateFailure) - sizeof(OperateFailure.szDescribeString);
    EmitDataBaseEngineResult(DBO_GP_OPERATE_FAILURE, dwContextID, &OperateFailure, wHeadSize + wDataSize);
  }
}

// 机器结果
VOID CDataBaseEngineSink::OnAndroidDisposeResult(DWORD dwContextID, DWORD dwErrorCode, WORD wSubCommdID, WORD wServerID) {
  // 构造结构
  DBO_GP_AndroidParameter AndroidParameter;
  ZeroMemory(&AndroidParameter, sizeof(AndroidParameter));

  // 设置变量
  AndroidParameter.wSubCommdID = wSubCommdID;
  AndroidParameter.wServerID = wServerID;

  if (dwErrorCode == DB_SUCCESS) {
    // 变量定义
    WORD wParameterCount = 0;

    while (!m_AccountsDBModule->IsRecordsetEnd()) {
      wParameterCount = AndroidParameter.wParameterCount;
      AndroidParameter.AndroidParameter[wParameterCount].dwBatchID = m_AccountsDBAide.GetValue_DWORD(TEXT("BatchID"));
      AndroidParameter.AndroidParameter[wParameterCount].dwAndroidCount = m_AccountsDBAide.GetValue_DWORD(TEXT("AndroidCount"));
      AndroidParameter.AndroidParameter[wParameterCount].dwServiceMode = m_AccountsDBAide.GetValue_DWORD(TEXT("ServiceMode"));
      AndroidParameter.AndroidParameter[wParameterCount].dwEnterTime = m_AccountsDBAide.GetValue_DWORD(TEXT("EnterTime"));
      AndroidParameter.AndroidParameter[wParameterCount].dwLeaveTime = m_AccountsDBAide.GetValue_DWORD(TEXT("LeaveTime"));
      AndroidParameter.AndroidParameter[wParameterCount].dwEnterMinInterval = m_AccountsDBAide.GetValue_DWORD(TEXT("EnterMinInterval"));
      AndroidParameter.AndroidParameter[wParameterCount].dwEnterMaxInterval = m_AccountsDBAide.GetValue_DWORD(TEXT("EnterMaxInterval"));
      AndroidParameter.AndroidParameter[wParameterCount].dwLeaveMinInterval = m_AccountsDBAide.GetValue_DWORD(TEXT("LeaveMinInterval"));
      AndroidParameter.AndroidParameter[wParameterCount].dwLeaveMaxInterval = m_AccountsDBAide.GetValue_DWORD(TEXT("LeaveMaxInterval"));
      AndroidParameter.AndroidParameter[wParameterCount].dwSwitchMinInnings = m_AccountsDBAide.GetValue_DWORD(TEXT("SwitchMinInnings"));
      AndroidParameter.AndroidParameter[wParameterCount].dwSwitchMaxInnings = m_AccountsDBAide.GetValue_DWORD(TEXT("SwitchMaxInnings"));
      AndroidParameter.AndroidParameter[wParameterCount].lTakeMinScore = m_AccountsDBAide.GetValue_LONGLONG(TEXT("TakeMinScore"));
      AndroidParameter.AndroidParameter[wParameterCount].lTakeMaxScore = m_AccountsDBAide.GetValue_LONGLONG(TEXT("TakeMaxScore"));
      AndroidParameter.AndroidParameter[wParameterCount].AndroidCountMember0 = m_AccountsDBAide.GetValue_DWORD(TEXT("AndroidCountMember0"));
      AndroidParameter.AndroidParameter[wParameterCount].AndroidCountMember1 = m_AccountsDBAide.GetValue_DWORD(TEXT("AndroidCountMember1"));
      AndroidParameter.AndroidParameter[wParameterCount].AndroidCountMember2 = m_AccountsDBAide.GetValue_DWORD(TEXT("AndroidCountMember2"));
      AndroidParameter.AndroidParameter[wParameterCount].AndroidCountMember3 = m_AccountsDBAide.GetValue_DWORD(TEXT("AndroidCountMember3"));
      AndroidParameter.AndroidParameter[wParameterCount].AndroidCountMember4 = m_AccountsDBAide.GetValue_DWORD(TEXT("AndroidCountMember4"));
      AndroidParameter.AndroidParameter[wParameterCount].AndroidCountMember5 = m_AccountsDBAide.GetValue_DWORD(TEXT("AndroidCountMember5"));

      // 设置变量
      AndroidParameter.wParameterCount++;

      // 溢出判断
      if (AndroidParameter.wParameterCount >= std::size(AndroidParameter.AndroidParameter)) {
        // 发送数据
        EmitDataBaseEngineResult(DBO_GP_ANDROID_PARAMETER, dwContextID, &AndroidParameter, sizeof(AndroidParameter));

        // 设置变量
        ZeroMemory(&AndroidParameter, sizeof(AndroidParameter));
      }

      // 移动游标
      m_AccountsDBModule->MoveToNext();
    }
  }

  // 计算大小
  WORD wSendDataSize = sizeof(AndroidParameter) - sizeof(AndroidParameter.AndroidParameter);
  wSendDataSize += AndroidParameter.wParameterCount * sizeof(tagAndroidParameter);

  // 发送数据
  EmitDataBaseEngineResult(DBO_GP_ANDROID_PARAMETER, dwContextID, &AndroidParameter, wSendDataSize);
}

VOID CDataBaseEngineSink::OnIndividualDisposeResult(DWORD dwContextID, DWORD dwErrorCode, LPCTSTR pszErrorString) {
  // 构造结构
  DBO_GP_IndividualResult individual;
  ZeroMemory(&individual, sizeof(individual));

  // 构造变量
  individual.bSuccessed = true;
  individual.lDiamond = m_AccountsDBAide.GetValue_LONGLONG(TEXT("Diamond"));
  LSTRCPYN(individual.szDescribeString, pszErrorString, std::size(individual.szDescribeString));

  // 发送结果
  WORD wDataSize = CountStringBuffer(individual.szDescribeString);
  WORD wHeadSize = sizeof(individual) - sizeof(individual.szDescribeString);
  EmitDataBaseEngineResult(DBO_GP_INDIVIDUAL_RESULT, dwContextID, &individual, wHeadSize + wDataSize);
}

// 获取约战房间游戏豆和房卡
bool CDataBaseEngineSink::OnRequestPersonalRoomUserInfo(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  try {
    DBR_GR_QUERY_PERSONAL_ROOM_USER_INFO* pUserInfo = (DBR_GR_QUERY_PERSONAL_ROOM_USER_INFO*)pData;
    ASSERT(wDataSize == sizeof(DBR_GR_QUERY_PERSONAL_ROOM_USER_INFO));
    if (wDataSize != sizeof(DBR_GR_QUERY_PERSONAL_ROOM_USER_INFO))
      return false;
    // 构造参数
    m_PlatformDBAide.ResetParameter();

    // 用户信息
    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pUserInfo->dwUserID);

    TCHAR szDescribeString[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GS_QueryPersonalRoomUserInfo"), true);
    if (DB_SUCCESS == lResultCode) {
      DBO_MB_PersonalRoomUserInfo userInfo;
      userInfo.lDiamond = m_PlatformDBAide.GetValue_LONGLONG(TEXT("Diamond"));
      // userInfo.dBeans = m_PlatformDBAide.GetValue_DOUBLE(TEXT("Beans"));

      EmitDataBaseEngineResult(DBO_GR_QUERY_PERSONAL_ROOM_USER_INFO_RESULT, dwContextID, &userInfo, sizeof(userInfo));
    }
  } catch (IDataBaseException* pIException) {
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

//-----------------------------------------------俱乐部-----------------
bool CDataBaseEngineSink::OnRequestCreateClub(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  ASSERT(wDataSize == sizeof(CMD_GP_Create_Club));
  if (wDataSize != sizeof(CMD_GP_Create_Club))
    return false;
  CMD_GP_Create_Club* pMsg = (CMD_GP_Create_Club*)pData;

  try {
    CLogger::Error(TEXT("收到请求创建俱乐部！"));
    // 构造参数
    m_AccountsDBAide.ResetParameter();

    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pMsg->dwUserID);
    m_AccountsDBAide.AddParameter(TEXT("@clubName"), pMsg->clubName);

    // 输出变量
    TCHAR szDescribe[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribe, sizeof(szDescribe), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GR_CreateClub"), true);

    CMD_GP_Create_Club_Result resultInfo;
    ZeroMemory(&resultInfo, sizeof(CMD_GP_Create_Club_Result));
    resultInfo.cbResult = lResultCode;

    if (lResultCode == DB_SUCCESS) {
      resultInfo.dwClubID = m_AccountsDBAide.GetValue_DWORD(TEXT("ClubID"));
      CLogger::Error(TEXT("创建俱乐部成功！"));
    } else {
      // 获取信息
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
      LSTRCPYN(resultInfo.szDescribeString, DBVarValue.c_str(), std::size(resultInfo.szDescribeString));
      CLogger::Error(TEXT("创建俱乐部失败：{}"), resultInfo.szDescribeString);
    }

    EmitDataBaseEngineResult(SUB_GP_CREATE_CLUB_RESULT, dwContextID, &resultInfo, sizeof(CMD_GP_Create_Club_Result));

  } catch (IDataBaseException* pIException) {
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

bool CDataBaseEngineSink::OnRequestGetClubList(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  ASSERT(wDataSize == sizeof(CMD_GP_GetClub_List));
  if (wDataSize != sizeof(CMD_GP_GetClub_List))
    return false;
  CMD_GP_GetClub_List* pMsg = (CMD_GP_GetClub_List*)pData;
  try {
    // updateUserOnLine(pMsg->dwUserID, 2);
    // CTraceService::TraceString(TEXT("收到请求获取俱乐部列表！"), TraceLevel_Exception);
    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pMsg->dwUserID);

    CMD_GP_Club_List kClub;
    ZeroMemory(&kClub, sizeof(CMD_GP_Club_List));

    // 执行查询
    if (DB_SUCCESS == m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GR_GetUserClubList"), true)) {
      while (!m_AccountsDBModule->IsRecordsetEnd()) {
        int clubID = m_AccountsDBAide.GetValue_DWORD(TEXT("ClubID"));

        TCHAR clubName[128]; //
        m_AccountsDBAide.GetValue_String(TEXT("ClubName"), clubName, std::size(clubName));

        int createID = m_AccountsDBAide.GetValue_DWORD(TEXT("CreateID"));
        int memberNum = m_AccountsDBAide.GetValue_DWORD(TEXT("MemberNum"));
        int diamond = m_AccountsDBAide.GetValue_DWORD(TEXT("Diamond"));

        m_AccountsDBAide.GetValue_String(TEXT("ClubInstruction"), kClub.clubInfo[kClub.clubCount].clubSummary,
                                         std::size(kClub.clubInfo[kClub.clubCount].clubSummary));

        /*char * clubName_1;
                                clubName_1 = GBKToUTF8(clubName);

                                TCHAR clubNameEx[32];
                                LSTRCPYN(kClub.clubInfo[kClub.clubCount].clubName, clubName_1, std::size(clubNameEx));

                                delete[] clubName_1;
                                clubName_1 = nullptr;*/

        LSTRCPYN(kClub.clubInfo[kClub.clubCount].clubName, clubName, std::size(clubName));

        m_AccountsDBAide.GetValue_String(TEXT("NickName"), kClub.clubInfo[kClub.clubCount].szNickName,
                                         std::size(kClub.clubInfo[kClub.clubCount].szNickName));

        kClub.clubInfo[kClub.clubCount].wFaceID = m_AccountsDBAide.GetValue_DWORD(TEXT("FaceID"));
        kClub.clubInfo[kClub.clubCount].dwCustomID = m_AccountsDBAide.GetValue_DWORD(TEXT("CustomID"));

        kClub.clubInfo[kClub.clubCount].dwClubID = clubID;
        kClub.clubInfo[kClub.clubCount].dwCreateUserID = createID;
        kClub.clubInfo[kClub.clubCount].memberNum = memberNum;
        kClub.clubInfo[kClub.clubCount].diamond = diamond;
        kClub.clubCount++;
        // 移动记录
        m_AccountsDBModule->MoveToNext();
      }
    }

    /*if (kClub.clubCount == 1)
                {
                        kClub.dwCurrClubID = kClub.clubInfo[0].dwClubID;
                }
                else if (kClub.clubCount > 1)
                {
                        kClub.dwCurrClubID = kClub.clubInfo[0].dwClubID;

                        m_AccountsDBAide.ResetParameter();
                        m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pMsg->dwUserID);

                        m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GR_QueryUserCurrClub"), true);

                        DWORD dwClubID = m_AccountsDBAide.GetValue_DWORD(TEXT("ClubID"));

                        if (dwClubID != 0)
                        {
                                kClub.dwCurrClubID = dwClubID;
                        }
                }*/
    // CTraceService::TraceString(TEXT("获取俱乐部列表成功！"), TraceLevel_Exception);
    EmitDataBaseEngineResult(SUB_GP_CLUB_LIST, dwContextID, &kClub, sizeof(CMD_GP_Club_List));
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

bool CDataBaseEngineSink::OnRequestGetClubUserList(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  ASSERT(wDataSize == sizeof(CMD_GP_GetClub_User_List));
  if (wDataSize != sizeof(CMD_GP_GetClub_User_List))
    return false;
  CMD_GP_GetClub_User_List* pMsg = (CMD_GP_GetClub_User_List*)pData;
  try {
    // updateUserOnLine(pMsg->dwUserID, 2);
    // 构造参数
    CLogger::Error(TEXT("收到请求获取俱乐部成员列表！"));
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwClubID"), pMsg->dwClubID);

    CMD_GP_Club_User_List_lua kClubUser;
    ZeroMemory(&kClubUser, sizeof(CMD_GP_Club_User_List_lua));
    kClubUser.index = pMsg->index;
    kClubUser.dwClubID = pMsg->dwClubID;

    // 执行查询
    if (DB_SUCCESS == m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GR_GetClubUserList"), true)) {
      int tNum = 0;
      while (!m_AccountsDBModule->IsRecordsetEnd()) {
        if (tNum >= 20 * pMsg->index)
          break;
        if (tNum >= 20 * (pMsg->index - 1) && tNum < 20 * pMsg->index) {
          CLogger::Error(TEXT("1111111！"));
          DWORD userID = m_AccountsDBAide.GetValue_DWORD(TEXT("UserID"));
          DWORD GameID = m_AccountsDBAide.GetValue_DWORD(TEXT("GameID"));
          CLogger::Error(TEXT("2222222！"));
          TCHAR nickName[128]; //
          m_AccountsDBAide.GetValue_String(TEXT("NickName"), kClubUser.userInfo[kClubUser.infoNum].szNickName,
                                           std::size(kClubUser.userInfo[kClubUser.infoNum].szNickName));
          CLogger::Error(TEXT("333333333！"));
          /*char * nickName_1;
                                        nickName_1 = GBKToUTF8(nickName);

                                        TCHAR nickNameEx[32];
                                        LSTRCPYN(kClubUser.userInfo[kClubUser.infoNum].szNickName, nickName_1, std::size(nickNameEx));

                                        delete[] nickName_1;
                                        nickName_1 = nullptr;*/

          // LSTRCPYN(kClubUser.userInfo[kClubUser.infoNum].szNickName, nickName, std::size(nickName));
          CLogger::Error(TEXT("4444444！"));
          DWORD dwUserStatus = m_AccountsDBAide.GetValue_DWORD(TEXT("UserStatus"));
          CLogger::Error(TEXT("55555555555！"));
          kClubUser.userInfo[kClubUser.infoNum].cbUserStatus = dwUserStatus;
          CLogger::Error(TEXT("66666666666！"));
          kClubUser.userInfo[kClubUser.infoNum].wFaceID = m_AccountsDBAide.GetValue_DWORD(TEXT("FaceID"));
          kClubUser.userInfo[kClubUser.infoNum].dwCustomID = m_AccountsDBAide.GetValue_DWORD(TEXT("CustomID"));
          CLogger::Error(TEXT("7777777777！"));
          kClubUser.userInfo[kClubUser.infoNum].dwUserID = userID;
          kClubUser.userInfo[kClubUser.infoNum].dwGameID = GameID;
          kClubUser.infoNum++;
          CLogger::Error(TEXT("8888888888！"));
        }
        tNum++;
        // 移动记录
        m_AccountsDBModule->MoveToNext();
      }
    }
    CLogger::Error(TEXT("获取俱乐部成员列表成功！"));
    EmitDataBaseEngineResult(SUB_GP_CLUB_USER_LIST_LUA, dwContextID, &kClubUser, sizeof(CMD_GP_Club_User_List_lua));
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

bool CDataBaseEngineSink::OnRequestGetClubName(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  ASSERT(wDataSize == sizeof(CMD_GP_Query_Club_Name));
  if (wDataSize != sizeof(CMD_GP_Query_Club_Name))
    return false;
  CMD_GP_Query_Club_Name* pMsg = (CMD_GP_Query_Club_Name*)pData;
  try {
    CLogger::Error(TEXT("收到请求查询俱乐部名称！"));
    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwClubID"), pMsg->dwClubID);

    CMD_GP_Query_Club_Name_Result clubResult;
    ZeroMemory(&clubResult, sizeof(CMD_GP_Query_Club_Name_Result));
    clubResult.dwClubID = pMsg->dwClubID;

    // 执行查询
    if (DB_SUCCESS == m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GR_QueryClubName"), true)) {
      CLogger::Error(TEXT("查询俱乐部名称成功！"));

      // TCHAR clubName[128];				//
      // m_AccountsDBAide.GetValue_String(TEXT("ClubName"), clubName, std::size(clubName));

      // char * clubName_1;
      // clubName_1 = GBKToUTF8(clubName);

      // TCHAR clubNameEx[32];
      // LSTRCPYN(clubResult.clubName, clubName_1, std::size(clubNameEx));

      // TCHAR nickName[128];				//
      // m_AccountsDBAide.GetValue_String(TEXT("NickName"), nickName, std::size(nickName));

      // char * nickName_1;
      // nickName_1 = GBKToUTF8(nickName);

      // TCHAR nickNameEx[32];
      // LSTRCPYN(clubResult.szNickName, nickName_1, std::size(nickNameEx));

      // delete[] clubName_1;
      // clubName_1 = nullptr;

      m_AccountsDBAide.GetValue_String(TEXT("ClubName"), clubResult.clubName, std::size(clubResult.clubName));
      m_AccountsDBAide.GetValue_String(TEXT("NickName"), clubResult.szNickName, std::size(clubResult.szNickName));
      clubResult.wFaceID = m_AccountsDBAide.GetValue_DWORD(TEXT("FaceID"));
      clubResult.dwCustomID = m_AccountsDBAide.GetValue_DWORD(TEXT("CustomID"));

      clubResult.cbResult = 0;

      clubResult.dwUserID = m_AccountsDBAide.GetValue_DWORD(TEXT("UserID"));
    } else {
      CLogger::Error(TEXT("没查到俱乐部名称！"));
      clubResult.cbResult = 1;
    }

    EmitDataBaseEngineResult(SUB_GP_QUERY_CLUB_NAME_RESULT, dwContextID, &clubResult, sizeof(CMD_GP_Query_Club_Name_Result));
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

bool CDataBaseEngineSink::OnRequestQueryJoinClub(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  ASSERT(wDataSize == sizeof(CMD_GP_Query_Join_Club));
  if (wDataSize != sizeof(CMD_GP_Query_Join_Club))
    return false;
  CMD_GP_Query_Join_Club* pMsg = (CMD_GP_Query_Join_Club*)pData;
  try {
    CLogger::Error(TEXT("收到请求加入俱乐部！"));
    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwClubID"), pMsg->dwClubID);
    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pMsg->dwUserID);
    // m_AccountsDBAide.AddParameter(TEXT("@szReason"), pMsg->szReason);

    CMD_GP_Query_Join_Club_Result joinClubResult;
    ZeroMemory(&joinClubResult, sizeof(CMD_GP_Query_Join_Club_Result));
    joinClubResult.dwClubID = pMsg->dwClubID;
    joinClubResult.dwUserID = pMsg->dwUserID;

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行查询
    if (DB_SUCCESS == m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GR_QueryJoinClub"), true)) {
      joinClubResult.cbResult = 0;
      CLogger::Error(TEXT("申请成功！"));
    } else {
      CLogger::Error(TEXT("申请失败！"));
      joinClubResult.cbResult = 1;
      // 结果处理
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
      LSTRCPYN(joinClubResult.szDescribeString, DBVarValue.c_str(), std::size(joinClubResult.szDescribeString));
    }

    EmitDataBaseEngineResult(SUB_GP_QUERY_JOIN_CLUB_RESULT, dwContextID, &joinClubResult,
                                                   sizeof(CMD_GP_Query_Join_Club_Result));
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

bool CDataBaseEngineSink::OnRequestQueryQuitClub(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  ASSERT(wDataSize == sizeof(CMD_GP_Query_Quit_Club));
  if (wDataSize != sizeof(CMD_GP_Query_Quit_Club))
    return false;
  CMD_GP_Query_Quit_Club* pMsg = (CMD_GP_Query_Quit_Club*)pData;
  try {
    CLogger::Error(TEXT("请求退出俱乐部！"));
    // 构造参数
    m_AccountsDBAide.ResetParameter();
    m_AccountsDBAide.AddParameter(TEXT("@dwClubID"), pMsg->dwClubID);
    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pMsg->dwUserID);

    CMD_GP_Query_Quit_Club_Result quitClubResult;
    ZeroMemory(&quitClubResult, sizeof(CMD_GP_Query_Quit_Club_Result));
    quitClubResult.dwClubID = pMsg->dwClubID;
    quitClubResult.dwUserID = pMsg->dwUserID;

    // 输出参数
    TCHAR szDescribeString[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribeString, sizeof(szDescribeString), nanodbc::statement::PARAM_OUT);

    // 执行查询
    if (DB_SUCCESS == m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GR_QueryQuitClub"), true)) {
      CLogger::Error(TEXT("退出俱乐部成功！"));
      quitClubResult.cbResult = 0;
    } else {
      CLogger::Error(TEXT("退出俱乐部失败！"));
      quitClubResult.cbResult = 1;
      // 结果处理
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
      LSTRCPYN(quitClubResult.szDescribeString, DBVarValue.c_str(), std::size(quitClubResult.szDescribeString));
    }

    EmitDataBaseEngineResult(SUB_GP_QUERY_QUIT_CLUB_RESULT, dwContextID, &quitClubResult,
                                                   sizeof(CMD_GP_Query_Quit_Club_Result));
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

bool CDataBaseEngineSink::OnRequestQueryClubJoinUserList(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  ASSERT(wDataSize == sizeof(CMD_GP_Query_Club_JoinUser_List));
  if (wDataSize != sizeof(CMD_GP_Query_Club_JoinUser_List))
    return false;
  CMD_GP_Query_Club_JoinUser_List* pMsg = (CMD_GP_Query_Club_JoinUser_List*)pData;

  try {
    // CTraceService::TraceString(TEXT("请求申请加入俱乐部的玩家列表！"), TraceLevel_Exception);
    // 构造参数
    m_AccountsDBAide.ResetParameter();

    m_AccountsDBAide.AddParameter(TEXT("@dwClubID"), pMsg->dwClubID);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GR_GetClubJoinUserList"), true);

    CMD_GP_Club_JoinUser_List info;
    ZeroMemory(&info, sizeof(CMD_GP_Club_JoinUser_List));
    info.dwClubID = pMsg->dwClubID;

    if (lResultCode == DB_SUCCESS) {
      // CTraceService::TraceString(TEXT("请求成功！"), TraceLevel_Exception);
      while (!m_AccountsDBModule->IsRecordsetEnd()) {
        if (info.num >= 40)
          break;

        DWORD userID = m_AccountsDBAide.GetValue_DWORD(TEXT("UserID"));
        DWORD GameID = m_AccountsDBAide.GetValue_DWORD(TEXT("GameID"));

        // TCHAR nickName[128];				//
        // m_AccountsDBAide.GetValue_String(TEXT("NickName"), nickName, std::size(nickName));

        // char * nickName_1;
        // nickName_1 = GBKToUTF8(nickName);

        // TCHAR nickNameEx[32];
        // LSTRCPYN(info.userInfo[info.num].szNickName, nickName_1, std::size(nickNameEx));

        // delete[] nickName_1;
        // nickName_1 = nullptr;

        m_AccountsDBAide.GetValue_String(TEXT("NickName"), info.userInfo[info.num].szNickName, std::size(info.userInfo[info.num].szNickName));
        info.userInfo[info.num].wFaceID = m_AccountsDBAide.GetValue_DWORD(TEXT("FaceID"));
        info.userInfo[info.num].dwCustomID = m_AccountsDBAide.GetValue_DWORD(TEXT("CustomID"));
        info.userInfo[info.num].dwUserID = userID;
        info.userInfo[info.num].dwGameID = GameID;
        info.num++;

        // 移动记录
        m_AccountsDBModule->MoveToNext();
      }
    }

    EmitDataBaseEngineResult(SUB_GP_CLUB_JOINUSER_LIST, dwContextID, &info, sizeof(CMD_GP_Club_JoinUser_List));

  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

bool CDataBaseEngineSink::OnRequestDealUserJoinClub(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  ASSERT(wDataSize == sizeof(CMD_GP_DealUserJoinClub));
  if (wDataSize != sizeof(CMD_GP_DealUserJoinClub))
    return false;
  CMD_GP_DealUserJoinClub* pMsg = (CMD_GP_DealUserJoinClub*)pData;

  try {
    CLogger::Error(TEXT("处理玩家加入俱乐部的请求！"));
    // 构造参数
    m_AccountsDBAide.ResetParameter();

    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pMsg->dwUserID);
    m_AccountsDBAide.AddParameter(TEXT("@dwClubID"), pMsg->dwClubID);
    m_AccountsDBAide.AddParameter(TEXT("@dwDuserID"), pMsg->dwDuserID);
    m_AccountsDBAide.AddParameter(TEXT("@cbDeal"), pMsg->cbDeal);

    // 输出变量
    TCHAR szDescribe[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribe, sizeof(szDescribe), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GR_DealUserJoinClub"), true);

    CMD_GP_DealUserJoinClubResult resultInfo;
    ZeroMemory(&resultInfo, sizeof(CMD_GP_DealUserJoinClubResult));
    resultInfo.cbResult = lResultCode;

    if (lResultCode == DB_SUCCESS) {

      CLogger::Error(TEXT("处理成功！"));
    } else {
      CLogger::Error(TEXT("处理失败！"));
      // 获取信息
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
      LSTRCPYN(resultInfo.szDescribeString, DBVarValue.c_str(), std::size(resultInfo.szDescribeString));
    }

    EmitDataBaseEngineResult(SUB_GP_DEAL_USER_JOIN_CLUB_RESULT, dwContextID, &resultInfo,
                                                   sizeof(CMD_GP_DealUserJoinClubResult));
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

bool CDataBaseEngineSink::OnRequestMoveJewelToClub(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  ASSERT(wDataSize == sizeof(CMD_GR_MoveJewelToClub));
  if (wDataSize != sizeof(CMD_GR_MoveJewelToClub))
    return false;
  CMD_GR_MoveJewelToClub* pMsg = (CMD_GR_MoveJewelToClub*)pData;

  try {
    CLogger::Error(TEXT("请求转移钻石到俱乐部！"));
    // 构造参数
    m_AccountsDBAide.ResetParameter();

    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pMsg->dwUserID);
    m_AccountsDBAide.AddParameter(TEXT("@dwClubID"), pMsg->dwClubID);
    m_AccountsDBAide.AddParameter(TEXT("@iNum"), pMsg->iNum);

    // 输出变量
    TCHAR szDescribe[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribe, sizeof(szDescribe), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GR_MoveJewelToClub"), true);

    CMD_GR_MoveJewelToClubResult resultInfo;
    ZeroMemory(&resultInfo, sizeof(CMD_GR_MoveJewelToClubResult));
    resultInfo.cbResult = lResultCode;

    if (lResultCode == DB_SUCCESS) {
      CLogger::Error(TEXT("转移钻石到俱乐部成功！"));
      resultInfo.iClubJewelNum = m_AccountsDBAide.GetValue_DWORD(TEXT("jewelNum"));
    } else {
      CLogger::Error(TEXT("转移钻石到俱乐部失败！"));
      // 获取信息
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
      LSTRCPYN(resultInfo.szDescribeString, DBVarValue.c_str(), std::size(resultInfo.szDescribeString));
    }

    EmitDataBaseEngineResult(SUB_GP_MOVE_JEWEL_TO_CLUB_RESULT, dwContextID, &resultInfo, sizeof(CMD_GR_MoveJewelToClubResult));

  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

bool CDataBaseEngineSink::OnRequestDeleteClubUser(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  ASSERT(wDataSize == sizeof(CMD_GR_DeleteClubUser));
  if (wDataSize != sizeof(CMD_GR_DeleteClubUser))
    return false;
  CMD_GR_DeleteClubUser* pMsg = (CMD_GR_DeleteClubUser*)pData;

  try {
    CLogger::Error(TEXT("请求删除俱乐部成员！"));
    // 构造参数
    m_AccountsDBAide.ResetParameter();

    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pMsg->dwUserID);
    m_AccountsDBAide.AddParameter(TEXT("@dwClubID"), pMsg->dwClubID);
    m_AccountsDBAide.AddParameter(TEXT("@dwDuserID"), pMsg->dwDuserID);

    // 输出变量
    TCHAR szDescribe[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribe, sizeof(szDescribe), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GR_DeleteClubUser"), true);

    CMD_GR_DeleteClubUserResult resultInfo;
    ZeroMemory(&resultInfo, sizeof(CMD_GR_DeleteClubUserResult));
    resultInfo.cbResult = lResultCode;

    if (lResultCode == DB_SUCCESS) {
      CLogger::Error(TEXT("删除俱乐部成员成功！"));
    } else {
      CLogger::Error(TEXT("删除俱乐部成员失败！"));
      // 获取信息
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
      LSTRCPYN(resultInfo.szDescribeString, DBVarValue.c_str(), std::size(resultInfo.szDescribeString));
    }

    EmitDataBaseEngineResult(SUB_GP_DELETE_CLUB_USER_RESULT, dwContextID, &resultInfo, sizeof(CMD_GR_DeleteClubUserResult));

  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

bool CDataBaseEngineSink::OnRequestCreateClubRoomRule(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  ASSERT(wDataSize == sizeof(CMD_GR_CreateClubRoomRule));
  if (wDataSize != sizeof(CMD_GR_CreateClubRoomRule))
    return false;
  CMD_GR_CreateClubRoomRule* pMsg = (CMD_GR_CreateClubRoomRule*)pData;

  try {
    // CTraceService::TraceString(TEXT("请求创建俱乐部规则！"), TraceLevel_Exception);
    // 构造参数
    m_PlatformDBAide.ResetParameter();

    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pMsg->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@dwClubID"), pMsg->dwClubID);
    m_PlatformDBAide.AddParameter(TEXT("@dwKindID"), pMsg->dwKindID);
    m_PlatformDBAide.AddParameter(TEXT("@dwDrawCountLimit"), pMsg->dwDrawCountLimit);
    m_PlatformDBAide.AddParameter(TEXT("@dwDrawTimeLimit"), pMsg->dwDrawTimeLimit);
    m_PlatformDBAide.AddParameter(TEXT("@lCellScore"), pMsg->lCellScore);
    m_PlatformDBAide.AddParameter(TEXT("@wJoinGamePeopleCount"), pMsg->wJoinGamePeopleCount);

    StringT str;
    for (BYTE i = 0; i < RULE_LEN; ++i) {
      str.append(fmt::format(TEXT("{}:"), pMsg->cbGameRule[i]));
    }
    // CTraceService::TraceString(TEXT("规则如下！"), TraceLevel_Exception);
    // CString tmpstr;
    // tmpstr.Format(TEXT("%s"), str);
    // CTraceService::TraceString(tmpstr, TraceLevel_Exception);
    m_PlatformDBAide.AddParameter(TEXT("@cbGameRule"), str.c_str());

    m_PlatformDBAide.AddParameter(TEXT("@cbGameMode"), pMsg->cbGameMode);

    // 输出变量
    TCHAR szDescribe[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribe, sizeof(szDescribe), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GR_CreateClubRoomRule"), true);

    CMD_GR_CreateClubRoomRuleResult resultInfo;
    ZeroMemory(&resultInfo, sizeof(CMD_GR_CreateClubRoomRuleResult));
    resultInfo.cbResult = lResultCode;

    if (lResultCode == DB_SUCCESS) {
      // CTraceService::TraceString(TEXT("创建俱乐部规则成功！"), TraceLevel_Exception);
    } else {
      // CTraceService::TraceString(TEXT("创建俱乐部规则失败！"), TraceLevel_Exception);
      // 获取信息
      StringT DBVarValue;
      m_PlatformDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
      LSTRCPYN(resultInfo.szDescribeString, DBVarValue.c_str(), std::size(resultInfo.szDescribeString));
    }

    EmitDataBaseEngineResult(SUB_GP_CREATE_CLUB_ROOM_RULE_RESULT, dwContextID, &resultInfo,
                                                   sizeof(CMD_GR_CreateClubRoomRuleResult));
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

bool CDataBaseEngineSink::OnRequestGetClubAllRoomRule(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  ASSERT(wDataSize == sizeof(CMD_GR_GetClubAllRoomRule));
  if (wDataSize != sizeof(CMD_GR_GetClubAllRoomRule))
    return false;
  CMD_GR_GetClubAllRoomRule* pMsg = (CMD_GR_GetClubAllRoomRule*)pData;

  try {
    // CTraceService::TraceString(TEXT("请求俱乐部所有规则！"), TraceLevel_Exception);
    // 构造参数
    m_PlatformDBAide.ResetParameter();

    m_PlatformDBAide.AddParameter(TEXT("@dwClubID"), pMsg->dwClubID);

    // 执行查询
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GR_GetClubAllRoomRule"), true);

    CMD_GR_ClubAllRoomRule info;
    ZeroMemory(&info, sizeof(CMD_GR_ClubAllRoomRule));
    info.dwClubID = pMsg->dwClubID;

    if (lResultCode == DB_SUCCESS) {
      // CTraceService::TraceString(TEXT("请求俱乐部所有规则成功！"), TraceLevel_Exception);
      while (!m_PlatformDBModule->IsRecordsetEnd()) {
        if (info.iNum >= 20)
          break;
        info.ruleInfo[info.iNum].dwKindID = m_PlatformDBAide.GetValue_DWORD(TEXT("KindID"));

        info.ruleInfo[info.iNum].dwDrawCountLimit = m_PlatformDBAide.GetValue_BYTE(TEXT("DrawCountLimit"));
        info.ruleInfo[info.iNum].dwDrawTimeLimit = m_PlatformDBAide.GetValue_BYTE(TEXT("DrawTimeLimit"));
        info.ruleInfo[info.iNum].lCellScore = m_PlatformDBAide.GetValue_DWORD(TEXT("CellScore"));
        info.ruleInfo[info.iNum].wJoinGamePeopleCount = m_PlatformDBAide.GetValue_DWORD(TEXT("JoinGamePeopleCount"));

        TCHAR str[400];
        m_PlatformDBAide.GetValue_String(TEXT("GameRule"), str, std::size(str));

        // CString tmpstr;
        // tmpstr.Format(TEXT("%s"), str);
        // CTraceService::TraceString(tmpstr, TraceLevel_Exception);

        std::vector<StringT> SubItemArray = StringUtils::Split(str, TEXT(':'));
        for (int i = 0; i < SubItemArray.size(); ++i) {
          info.ruleInfo[info.iNum].cbGameRule[i] = std::stoi(SubItemArray[i]);
          CLogger::Info(TEXT("规则{}={}"), i + 1, (int)info.ruleInfo[info.iNum].cbGameRule[i]);
        }
        info.ruleInfo[info.iNum].cbGameMode = m_PlatformDBAide.GetValue_DWORD(TEXT("GameMode"));

        CLogger::Info(TEXT("游戏节点={}"), (int)info.ruleInfo[info.iNum].cbGameMode);

        info.iNum++;

        // 移动记录
        m_PlatformDBModule->MoveToNext();
      }
    }

    EmitDataBaseEngineResult(SUB_GP_CLUB_ALL_ROOM_RULE, dwContextID, &info, sizeof(CMD_GR_ClubAllRoomRule));

  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

bool CDataBaseEngineSink::OnRequestDeleteClubRoomRule(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  ASSERT(wDataSize == sizeof(CMD_GR_DeleteClubRoomRule));
  if (wDataSize != sizeof(CMD_GR_DeleteClubRoomRule))
    return false;
  CMD_GR_DeleteClubRoomRule* pMsg = (CMD_GR_DeleteClubRoomRule*)pData;

  try {
    CLogger::Error(TEXT("请求删除俱乐部规则！"));
    // 构造参数
    m_PlatformDBAide.ResetParameter();

    m_PlatformDBAide.AddParameter(TEXT("@dwUserID"), pMsg->dwUserID);
    m_PlatformDBAide.AddParameter(TEXT("@dwClubID"), pMsg->dwClubID);
    m_PlatformDBAide.AddParameter(TEXT("@dwKindID"), pMsg->dwKindID);
    m_PlatformDBAide.AddParameter(TEXT("@dwDrawCountLimit"), pMsg->dwDrawCountLimit);
    m_PlatformDBAide.AddParameter(TEXT("@dwDrawTimeLimit"), pMsg->dwDrawTimeLimit);
    m_PlatformDBAide.AddParameter(TEXT("@lCellScore"), pMsg->lCellScore);
    m_PlatformDBAide.AddParameter(TEXT("@wJoinGamePeopleCount"), pMsg->wJoinGamePeopleCount);

    StringT str;
    for (BYTE i = 0; i < RULE_LEN; ++i) {
      str.append(fmt::format(TEXT("{}:"), pMsg->cbGameRule[i]));
    }
    // CTraceService::TraceString(TEXT("规则如下！"), TraceLevel_Exception);
    // CString tmpstr;
    // tmpstr.Format(TEXT("%s"),str);
    // CTraceService::TraceString(tmpstr, TraceLevel_Exception);
    m_PlatformDBAide.AddParameter(TEXT("@cbGameRule"), str.c_str());
    m_PlatformDBAide.AddParameter(TEXT("@cbGameMode"), pMsg->cbGameMode);

    // 输出变量
    TCHAR szDescribe[128] = TEXT("");
    m_PlatformDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribe, sizeof(szDescribe), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GR_DeleteClubRoomRule"), true);

    CMD_GR_DeleteClubRoomRuleResult resultInfo;
    ZeroMemory(&resultInfo, sizeof(CMD_GR_DeleteClubRoomRuleResult));
    resultInfo.cbResult = lResultCode;

    if (lResultCode == DB_SUCCESS) {
      // CTraceService::TraceString(TEXT("删除俱乐部规则成功！"), TraceLevel_Exception);
    } else {
      // CTraceService::TraceString(TEXT("删除俱乐部规则失败！"), TraceLevel_Exception);
      // 获取信息
      StringT DBVarValue;
      m_PlatformDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
      LSTRCPYN(resultInfo.szDescribeString, DBVarValue.c_str(), std::size(resultInfo.szDescribeString));
    }

    EmitDataBaseEngineResult(SUB_GP_DELETE_CLUB_ROOM_RULE_RESULT, dwContextID, &resultInfo,
                                                   sizeof(CMD_GR_DeleteClubRoomRuleResult));

  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

bool CDataBaseEngineSink::OnRequestGetClubRoomList(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  ASSERT(wDataSize == sizeof(CMD_GP_GetClub_Room_List));
  if (wDataSize != sizeof(CMD_GP_GetClub_Room_List))
    return false;
  CMD_GP_GetClub_Room_List* pMsg = (CMD_GP_GetClub_Room_List*)pData;
  try {
    //-------------------------------发送该俱乐部前10个房间-----------------优先等待中的
    CMD_GP_Club_Room_List_lua roomListLua;
    ZeroMemory(&roomListLua, sizeof(CMD_GP_Club_Room_List_lua));
    roomListLua.dwClubID = pMsg->dwClubID;

    m_PlatformDBAide.ResetParameter();
    m_PlatformDBAide.AddParameter(TEXT("@dwClubID"), pMsg->dwClubID);
    m_PlatformDBAide.AddParameter(TEXT("@dwKindID"), pMsg->dwKindID);
    m_PlatformDBAide.AddParameter(TEXT("@dwDrawCountLimit"), pMsg->dwDrawCountLimit);
    m_PlatformDBAide.AddParameter(TEXT("@dwDrawTimeLimit"), pMsg->dwDrawTimeLimit);
    m_PlatformDBAide.AddParameter(TEXT("@lCellScore"), pMsg->lCellScore);
    m_PlatformDBAide.AddParameter(TEXT("@wJoinGamePeopleCount"), pMsg->wJoinGamePeopleCount);

    StringT str;
    for (BYTE i = 0; i < RULE_LEN; ++i) {
      str.append(fmt::format(TEXT("{}:"), pMsg->cbGameRule[i]));
    }
    /*CTraceService::TraceString(TEXT("规则如下！"), TraceLevel_Exception);
                CString tmpstr;
                tmpstr.Format(TEXT("%s"), str);
                CTraceService::TraceString(tmpstr, TraceLevel_Exception);*/
    m_PlatformDBAide.AddParameter(TEXT("@cbGameRule"), str.c_str());
    m_PlatformDBAide.AddParameter(TEXT("@cbGameMode"), pMsg->cbGameMode);

    // 执行查询
    if (DB_SUCCESS == m_PlatformDBAide.ExecuteProcess(TEXT("GSP_GR_QueryClubCreataPrivateRoom"), true)) {
      roomListLua.index = pMsg->index;
      int tNum = 0;
      while (!m_PlatformDBModule->IsRecordsetEnd()) {
        if (tNum >= 10 * (pMsg->index - 1) && tNum < 10 * pMsg->index) {

          roomListLua.roomInfo[roomListLua.roomNum].dwRoomNum = m_PlatformDBAide.GetValue_DWORD(TEXT("RoomID"));
          roomListLua.roomInfo[roomListLua.roomNum].dwKindID = m_PlatformDBAide.GetValue_DWORD(TEXT("KindID"));
          roomListLua.roomInfo[roomListLua.roomNum].dwCreateGameID = m_PlatformDBAide.GetValue_DWORD(TEXT("GameID"));
          roomListLua.roomInfo[roomListLua.roomNum].wFaceID = m_PlatformDBAide.GetValue_DWORD(TEXT("FaceID"));
          roomListLua.roomInfo[roomListLua.roomNum].dwCustomID = m_PlatformDBAide.GetValue_DWORD(TEXT("CustomID"));
          m_PlatformDBAide.GetValue_String(TEXT("NickName"), roomListLua.roomInfo[roomListLua.roomNum].createNickName,
                                           std::size(roomListLua.roomInfo[roomListLua.roomNum].createNickName));

          roomListLua.roomInfo[roomListLua.roomNum].dwDrawCountLimit = m_PlatformDBAide.GetValue_DWORD(TEXT("DrawCountLimit"));
          roomListLua.roomInfo[roomListLua.roomNum].dwDrawTimeLimit = m_PlatformDBAide.GetValue_DWORD(TEXT("DrawTimeLimit"));
          roomListLua.roomInfo[roomListLua.roomNum].lCellScore = m_PlatformDBAide.GetValue_DWORD(TEXT("CellScore"));
          roomListLua.roomInfo[roomListLua.roomNum].wJoinGamePeopleCount = m_PlatformDBAide.GetValue_DWORD(TEXT("JoinGamePeopleCount"));

          TCHAR str[400]{};
          m_PlatformDBAide.GetValue_String(TEXT("GameRule"), str, std::size(str));
          // CString tmpstr;
          // tmpstr.Format(TEXT("%s"), str);
          // CTraceService::TraceString(tmpstr, TraceLevel_Exception);

          std::vector<StringT> SubItemArray = StringUtils::Split(str, TEXT(':'));
          for (int i = 0; i < SubItemArray.size(); ++i) {
            roomListLua.roomInfo[roomListLua.roomNum].cbGameRule[i] = std::stoi(SubItemArray[i]);
          }

          roomListLua.roomInfo[roomListLua.roomNum].cbUserCount = m_PlatformDBAide.GetValue_DWORD(TEXT("bUserCount"));
          roomListLua.roomInfo[roomListLua.roomNum].cbGameStart = m_PlatformDBAide.GetValue_DWORD(TEXT("bStartGame"));
          roomListLua.roomInfo[roomListLua.roomNum].cbCurrJu = m_PlatformDBAide.GetValue_DWORD(TEXT("currJu"));

          roomListLua.roomInfo[roomListLua.roomNum].userInfo[0].dwUserID = m_PlatformDBAide.GetValue_DWORD(TEXT("PlayUserID1"));
          roomListLua.roomInfo[roomListLua.roomNum].userInfo[1].dwUserID = m_PlatformDBAide.GetValue_DWORD(TEXT("PlayUserID2"));
          roomListLua.roomInfo[roomListLua.roomNum].userInfo[2].dwUserID = m_PlatformDBAide.GetValue_DWORD(TEXT("PlayUserID3"));
          roomListLua.roomInfo[roomListLua.roomNum].userInfo[3].dwUserID = m_PlatformDBAide.GetValue_DWORD(TEXT("PlayUserID4"));
          roomListLua.roomInfo[roomListLua.roomNum].userInfo[4].dwUserID = m_PlatformDBAide.GetValue_DWORD(TEXT("PlayUserID5"));
          roomListLua.roomInfo[roomListLua.roomNum].userInfo[5].dwUserID = m_PlatformDBAide.GetValue_DWORD(TEXT("PlayUserID6"));
          roomListLua.roomInfo[roomListLua.roomNum].userInfo[6].dwUserID = m_PlatformDBAide.GetValue_DWORD(TEXT("PlayUserID7"));
          roomListLua.roomInfo[roomListLua.roomNum].userInfo[7].dwUserID = m_PlatformDBAide.GetValue_DWORD(TEXT("PlayUserID8"));

          roomListLua.roomInfo[roomListLua.roomNum].userInfo[0].lUserScore = m_PlatformDBAide.GetValue_DWORD(TEXT("Score1"));
          roomListLua.roomInfo[roomListLua.roomNum].userInfo[1].lUserScore = m_PlatformDBAide.GetValue_DWORD(TEXT("Score2"));
          roomListLua.roomInfo[roomListLua.roomNum].userInfo[2].lUserScore = m_PlatformDBAide.GetValue_DWORD(TEXT("Score3"));
          roomListLua.roomInfo[roomListLua.roomNum].userInfo[3].lUserScore = m_PlatformDBAide.GetValue_DWORD(TEXT("Score4"));
          roomListLua.roomInfo[roomListLua.roomNum].userInfo[4].lUserScore = m_PlatformDBAide.GetValue_DWORD(TEXT("Score5"));
          roomListLua.roomInfo[roomListLua.roomNum].userInfo[5].lUserScore = m_PlatformDBAide.GetValue_DWORD(TEXT("Score6"));
          roomListLua.roomInfo[roomListLua.roomNum].userInfo[6].lUserScore = m_PlatformDBAide.GetValue_DWORD(TEXT("Score7"));
          roomListLua.roomInfo[roomListLua.roomNum].userInfo[7].lUserScore = m_PlatformDBAide.GetValue_DWORD(TEXT("Score8"));

          for (BYTE i = 0; i < 8; ++i) {
            if (roomListLua.roomInfo[roomListLua.roomNum].userInfo[i].dwUserID != 0) {
              m_AccountsDBAide.ResetParameter();
              m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), roomListLua.roomInfo[roomListLua.roomNum].userInfo[i].dwUserID);

              if (m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GP_GetClubRoomUserInfo"), false) == DB_SUCCESS) {
                roomListLua.roomInfo[roomListLua.roomNum].userInfo[i].dwGameID = m_AccountsDBAide.GetValue_DWORD(TEXT("GameID"));
                roomListLua.roomInfo[roomListLua.roomNum].userInfo[i].wFaceID = m_AccountsDBAide.GetValue_DWORD(TEXT("FaceID"));
                roomListLua.roomInfo[roomListLua.roomNum].userInfo[i].dwCustomID = m_AccountsDBAide.GetValue_DWORD(TEXT("CustomID"));
                m_AccountsDBAide.GetValue_String(TEXT("NickName"), roomListLua.roomInfo[roomListLua.roomNum].userInfo[i].szNickName,
                                                 std::size(roomListLua.roomInfo[roomListLua.roomNum].userInfo[i].szNickName));
              }
            }
          }
          roomListLua.roomNum++;
        }
        tNum++;
        // 移动记录
        m_PlatformDBModule->MoveToNext();
      }

      roomListLua.allRoomNum = tNum;
      EmitDataBaseEngineResult(SUB_GP_CLUB_ROOM_LIST, dwContextID, &roomListLua, sizeof(CMD_GP_Club_Room_List_lua));
    }
  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

bool CDataBaseEngineSink::OnRequestModifyClubSummary(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  ASSERT(wDataSize == sizeof(CMD_GR_ModifyClubSummary));
  if (wDataSize != sizeof(CMD_GR_ModifyClubSummary))
    return false;
  CMD_GR_ModifyClubSummary* pMsg = (CMD_GR_ModifyClubSummary*)pData;

  try {
    // 构造参数
    m_AccountsDBAide.ResetParameter();

    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pMsg->dwUserID);
    m_AccountsDBAide.AddParameter(TEXT("@dwClubID"), pMsg->dwClubID);
    m_AccountsDBAide.AddParameter(TEXT("@clubSummary"), pMsg->clubSummary);

    // 输出变量
    TCHAR szDescribe[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribe, sizeof(szDescribe), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GR_ModifyClubSummary"), true);

    CMD_GR_ModifyClubSummaryResult resultInfo;
    ZeroMemory(&resultInfo, sizeof(CMD_GR_ModifyClubSummaryResult));
    resultInfo.cbResult = lResultCode;

    if (lResultCode == DB_SUCCESS) {

    } else {
      // 获取信息
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
      LSTRCPYN(resultInfo.szDescribeString, DBVarValue.c_str(), std::size(resultInfo.szDescribeString));
    }

    EmitDataBaseEngineResult(SUB_GP_MODIFY_CLUB_SUMMARY_RESULT, dwContextID, &resultInfo,
                                                   sizeof(CMD_GR_ModifyClubSummaryResult));

  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

bool CDataBaseEngineSink::OnRequestJoinUserToClub(DWORD dwContextID, VOID* pData, WORD wDataSize) {
  ASSERT(wDataSize == sizeof(CMD_GR_JoinUserToClub));
  if (wDataSize != sizeof(CMD_GR_JoinUserToClub))
    return false;
  CMD_GR_JoinUserToClub* pMsg = (CMD_GR_JoinUserToClub*)pData;

  try {
    // 构造参数
    m_AccountsDBAide.ResetParameter();

    m_AccountsDBAide.AddParameter(TEXT("@dwUserID"), pMsg->dwUserID);
    m_AccountsDBAide.AddParameter(TEXT("@dwClubID"), pMsg->dwClubID);
    m_AccountsDBAide.AddParameter(TEXT("@dwGameID"), pMsg->dwGameID);

    // 输出变量
    TCHAR szDescribe[128] = TEXT("");
    m_AccountsDBAide.AddParameterOutput(TEXT("@strErrorDescribe"), szDescribe, sizeof(szDescribe), nanodbc::statement::PARAM_OUT);

    // 执行查询
    LONG lResultCode = m_AccountsDBAide.ExecuteProcess(TEXT("GSP_GR_JoinUserToClub"), true);

    CMD_GR_JoinUserToClubResult resultInfo;
    ZeroMemory(&resultInfo, sizeof(CMD_GR_JoinUserToClubResult));
    resultInfo.cbResult = lResultCode;

    if (lResultCode == DB_SUCCESS) {

    } else {
      // 获取信息
      StringT DBVarValue;
      m_AccountsDBModule->GetParameter(TEXT("@strErrorDescribe"), DBVarValue);
      LSTRCPYN(resultInfo.szDescribeString, DBVarValue.c_str(), std::size(resultInfo.szDescribeString));
    }

    EmitDataBaseEngineResult(SUB_GP_MODIFY_CLUB_SUMMARY_RESULT, dwContextID, &resultInfo,
                                                   sizeof(CMD_GR_ModifyClubSummaryResult));

  } catch (IDataBaseException* pIException) {
    // 输出错误
    StringT strDesc = pIException->GetExceptionDescribe();
    CLogger::Error(TEXT("{} → {}"), TEXT(__FUNCTION__), strDesc);
    return false;
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////////////
