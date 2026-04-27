#include "LogonSessionLogic.h"

#include "GlobalDefine/Packet.h"
#include "KernelEngine/TraceService.h"
#include "MessageDefine/CMD_LogonServer.h"

asio::awaitable<void> LogonSessionLogic::Prepare(const std::shared_ptr<SessionContext>& ctx) {
  // 订阅事件
  co_await Subscribe(MAKELONG(MDM_MB_LOGON, SUB_MB_LOGON_SUCCESS), &LogonSessionLogic::MbLogonSuccess, this);
  co_await Subscribe(MAKELONG(MDM_MB_LOGON, SUB_MB_LOGON_FAILURE), &LogonSessionLogic::MbLogonFailure, this);

  // 发送校验信息
  constexpr TCP_Validate validate{.szValidateKey = TEXT("8C3AC3BC-EB40-462f-B436-4BBB141FC7F9")};
  co_await Sending(MDM_KN_COMMAND, SUB_KN_VALIDATE_SOCKET, validate);

  // 发送登录信息
  CMD_MB_LogonOtherPlatform lop{
      .wModuleID = 65535,
      .dwPlazaVersion = 0x607'0001,
      .cbDeviceType = 1,
      .cbGender = 1,
      .cbPlatformID = 5,
      .szUserUin = TEXT("WBCasdasd4Asdjky"),
      .szNickName = TEXT("荣耀Wdw3"),
      .szCompellation = TEXT("荣耀Wdw3"),
      .szMachineID = TEXT("A501164B366ECFC9E249163873094D51"),
      .szMobilePhone = TEXT("0123456789"),
      .szDeviceToken = TEXT("91C6E897E346C74CE8D1DB0AC06E2C13"),
      .strFaceUrl = TEXT("http://jh.foxuc.net/image/custom.png"),
  };
  co_await Sending(MDM_MB_LOGON, SUB_MB_LOGON_OTHERPLATFORM, lop);
  co_return;
}

asio::awaitable<void> LogonSessionLogic::MbLogonSuccess(const std::vector<uint8_t>& data) {
  const auto msg = reinterpret_cast<const CMD_MB_LogonSuccess*>(data.data());
  CLogger::Info("当前登录用户：{}", ToSimpleUtf8(msg->szNickName));
  co_await Closing();
  co_return;
}

asio::awaitable<void> LogonSessionLogic::MbLogonFailure(const std::vector<uint8_t>& data) {
  const auto msg = reinterpret_cast<const CMD_MB_LogonFailure*>(data.data());
  CLogger::Info("{} 登录失败：{}", Context()->name, ToSimpleUtf8(msg->szDescribeString));
  co_return;
}
