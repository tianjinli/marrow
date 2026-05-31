#include "ServerControlItemSink.h"
#include "CommonDefine/Platform.h"

// 建立对象函数
extern "C" PLATFORM_EXPORT void* CreateServerControl() {
  return new (std::nothrow) CServerControlItemSink();
}

// 服务器控制
bool CServerControlItemSink::ServerControl(CMD_S_CheatCard* pCheatCard, ITableFrame* pITableFrame) {
  for (WORD i = 0; i < GAME_PLAYER; i++) {
    IServerUserItem* pIServerUserItem = pITableFrame->GetTableUserItem(i);
    ASSERT(pIServerUserItem);
    if (pIServerUserItem) {
      // 作弊用户
      if (CUserRight::IsGameCheatUser(pIServerUserItem->GetUserRight())) {
        ServerControl(pCheatCard, pITableFrame, pIServerUserItem);
      }
    }
  }

  // WB改
  WORD wEnumIndex = 0;
  do {
    IServerUserItem* pIServerUserItem = pITableFrame->EnumLookonUserItem(wEnumIndex++);
    if (pIServerUserItem == nullptr) {
      break;
    }
    // 作弊用户
    if (CUserRight::IsGameCheatUser(pIServerUserItem->GetUserRight())) {
      ServerControl(pCheatCard, pITableFrame, pIServerUserItem);
    }
  } while (TRUE);

  return true;
}

// 服务器控制
bool CServerControlItemSink::ServerControl(CMD_S_CheatCard* pCheatCard, ITableFrame* pITableFrame, IServerUserItem* pIServerUserItem) {
  pITableFrame->SendUserItemData(pIServerUserItem, SUB_S_CHEAT_CARD, pCheatCard, sizeof(CMD_S_CheatCard));
  return true;
}
