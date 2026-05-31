#pragma once

// 游戏控制基类
class IServerControl {
public:
  IServerControl() = default;
  virtual ~IServerControl() = default;

public:
  // 服务器控制
  virtual bool ServerControl(CMD_S_CheatCard* pCheatCard, ITableFrame* pITableFrame) = 0;
  virtual bool ServerControl(CMD_S_CheatCard* pCheatCard, ITableFrame* pITableFrame, IServerUserItem* pIServerUserItem) = 0;
};
