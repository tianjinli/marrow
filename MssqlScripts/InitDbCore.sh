#!/usr/bin/env bash
set -euo pipefail

# 尽量保证中文文件名可用
export LANG=C.UTF-8
export LC_ALL=C.UTF-8

DB_HOST=${DB_HOST:-localhost}
DB_USER=${DB_USER:-sa}
DB_PASS=${DB_PASS:-poTP9YjMrLhVhm5B}
NO_PAUSE=${NO_PAUSE:-'0'}

# 工作目录：脚本所在目录
workDir="$(cd "$(dirname "$0")" && pwd)"
sqlcmdDir="$workDir/mssql-tools18"
export PATH="$PATH:$sqlcmdDir:$workDir"

# 终端标题（可选）
printf '\033]0;%s\007' "网狐棋牌精华版一键安装"
echo "当前命令行参数：-S $DB_HOST -U $DB_USER -P $DB_PASS -C -i"

sysOS=$(uname -s)
sysArch=$(uname -m)

run_sql() {
  local file=$1
  # -b：遇到错误退出非零状态；-l 超时；-i 输入文件
  sqlcmd -S $DB_HOST -U $DB_USER -P $DB_PASS -C -i "$file"
}

pause() {
  local msg="${1:-按回车继续...}"
  if [[ "$NO_PAUSE" == "0" ]]; then
    read -r -p "$msg"
  fi
}

log() {
  # 加时间戳日志
  printf '[%s] %s\n' "$(date '+%F %T')" "$*"
}

install_bzip2() {
  if command -v bzip2 >/dev/null 2>&1; then
    echo "✅ bzip2 已安装"
    return
  fi

  case "$sysOS" in
    Darwin)
      if ! command -v brew >/dev/null 2>&1; then
        echo "❌ Homebrew 未安装，请先安装 Homebrew"
        exit 1
      fi
      brew install bzip2
      ;;
    Linux)
      if command -v apt-get >/dev/null 2>&1; then
        sudo apt-get update -y
        sudo apt-get install -y bzip2
      elif command -v yum >/dev/null 2>&1; then
        sudo yum install -y bzip2
      elif command -v dnf >/dev/null 2>&1; then
        sudo dnf install -y bzip2
      else
        echo "❌ 未找到可用的包管理器，请手动安装 bzip2"
        exit 1
      fi
      ;;
    *)
      echo "❌ 不支持的系统类型: $sysOS"
      exit 1
      ;;
  esac
}

function download_filename() {
  case "$sysOS" in
    Darwin)
      case "$sysArch" in
        arm64) echo "sqlcmd-macos-arm64.tar.bz2" ;;
        x86_64) echo "sqlcmd-macos-x64.tar.bz2" ;;
        *) echo "❌ 不支持的 macOS 架构: $sysArch"; exit 1 ;;
      esac
      ;;
    Linux)
      echo "sqlcmd-linux-amd64.tar.bz2"
      ;;
    *)
      echo "❌ 不支持的系统类型: $sysOS"
      exit 1
      ;;
  esac
}

# 如果没有安装 bzip2
install_bzip2

# 检查是否已存在
if command -v sqlcmd >/dev/null 2>&1 || [[ -x "$sqlcmdDir/sqlcmd" ]]; then
  echo "✅ sqlcmd 已存在，跳过下载。"
else
  baseUrl="https://github.com/microsoft/go-sqlcmd/releases/download/v1.8.2"

  # 目录不存在则创建
  mkdir -p "$sqlcmdDir"
  downloadPath="$sqlcmdDir/$(download_filename)"
  downloadUrl="$baseUrl/$(download_filename)"

  echo "⬇️ 正在下载 $downloadUrl ..."
  curl -L "$downloadUrl" -o "$downloadPath"

  echo "📦 正在解压到 $sqlcmdDir ..."
  echo ar -xjf "$downloadPath" -C "$sqlcmdDir"
  tar -xjf "$downloadPath" -C "$sqlcmdDir"

  echo "🗑 删除压缩包 ..."
  rm -f "$downloadPath"

  echo "✅ 安装完成，sqlcmd 位于 $sqlcmdDir"
  echo "   运行方式：$sqlcmdDir/sqlcmd"

fi

main() {
  echo "*************************************************************************"
  echo "          执行网狐棋牌精华版数据库一键安装脚本，自动创建初始数据库。          "
  echo "*************************************************************************"
  pause

  echo ""
  echo "删除数据库"
  rootPath="$workDir/1_1创建数据库"
  run_sql "$rootPath/数据库删除.sql"

  echo ""
  log "建立数据库"
  run_sql "$rootPath/1_1_用户库脚本.sql"
  run_sql "$rootPath/1_2_平台库脚本.sql"
  run_sql "$rootPath/1_3_金币库脚本.sql"
  run_sql "$rootPath/1_4_记录库脚本.sql"
  run_sql "$rootPath/1_5_积分库脚本.sql"
  run_sql "$rootPath/1_6_比赛库脚本.sql"
  run_sql "$rootPath/1_7_练习库脚本.sql"

  run_sql "$rootPath/2_1_用户库脚本.sql"
  run_sql "$rootPath/2_2_平台库脚本.sql"
  run_sql "$rootPath/2_3_金币库脚本.sql"
  run_sql "$rootPath/2_4_记录库脚本.sql"
  run_sql "$rootPath/2_5_积分库脚本.sql"
  run_sql "$rootPath/2_6_比赛库脚本.sql"
  run_sql "$rootPath/2_7_练习库脚本.sql"

  echo ""
  echo "建立链接服务器"
  rootPath="$workDir/1_2创建链接服务器"
  run_sql "$rootPath/1_1用户链接.sql"
  run_sql "$rootPath/1_2平台链接.sql"
  run_sql "$rootPath/1_3金币链接.sql"
  run_sql "$rootPath/1_4记录链接.sql"
  run_sql "$rootPath/1_5积分链接.sql"
  run_sql "$rootPath/1_6比赛链接.sql"
  run_sql "$rootPath/1_7练习链接.sql"

  echo ""
  echo "建立初始数据"

  rootPath="$workDir/1_3创建初始数据/1_1用户初始数据"
  run_sql "$rootPath/系统配置.sql"
  run_sql "$rootPath/会员配置.sql"

  rootPath="$workDir/1_3创建初始数据/1_2平台初始数据"
  run_sql "$rootPath/道具关系.sql"
  run_sql "$rootPath/道具类型.sql"
  run_sql "$rootPath/道具配置.sql"
  run_sql "$rootPath/类型配置.sql"
  run_sql "$rootPath/签到配置.sql"
  run_sql "$rootPath/子道具配置.sql"

  rootPath="$workDir/1_3创建初始数据/1_3金币初始数据"
  run_sql "$rootPath/返利配置.sql"
  run_sql "$rootPath/列表配置.sql"

  rootPath="$workDir/1_3创建初始数据/1_5积分初始数据"
  run_sql "$rootPath/列表配置.sql"

  rootPath="$workDir/1_3创建初始数据/1_6比赛初始数据"
  run_sql "$rootPath/列表配置.sql"

  rootPath="$workDir/1_3创建初始数据/1_7练习初始数据"
  run_sql "$rootPath/列表配置.sql"

  echo ""
  echo "建立存储过程"

  rootPath="$workDir/1_4创建存储过程/1_1用户数据库"
  run_sql "$rootPath/绑定机器.sql"
  run_sql "$rootPath/标识登录.sql"
  run_sql "$rootPath/代理列表.sql"
  run_sql "$rootPath/好友操作.sql"
  run_sql "$rootPath/好友查找.sql"
  run_sql "$rootPath/好友登录.sql"
  run_sql "$rootPath/好友消息.sql"
  run_sql "$rootPath/机器管理.sql"
  run_sql "$rootPath/加载机器.sql"
  run_sql "$rootPath/设置权限.sql"
  run_sql "$rootPath/实名验证.sql"
  run_sql "$rootPath/校验权限.sql"
  run_sql "$rootPath/校验资料.sql"
  run_sql "$rootPath/修改密码.sql"
  run_sql "$rootPath/修改签名.sql"
  run_sql "$rootPath/用户资料.sql"
  run_sql "$rootPath/帐号绑定.sql"
  run_sql "$rootPath/帐号登录.sql"
  run_sql "$rootPath/注册帐号.sql"
  run_sql "$rootPath/自定头像.sql"

  rootPath="$workDir/1_4创建存储过程/1_2平台数据库"
  run_sql "$rootPath/背包管理.sql"
  run_sql "$rootPath/道具管理.sql"
  run_sql "$rootPath/房间管理.sql"
  run_sql "$rootPath/会员管理.sql"
  run_sql "$rootPath/加载节点.sql"
  run_sql "$rootPath/加载类型.sql"
  run_sql "$rootPath/加载敏感词.sql"
  run_sql "$rootPath/加载页面.sql"
  run_sql "$rootPath/加载种类.sql"
  run_sql "$rootPath/喇叭使用.sql"
  run_sql "$rootPath/连接信息.sql"
  run_sql "$rootPath/加载消息.sql"
  run_sql "$rootPath/模块管理.sql"
  run_sql "$rootPath/平台配置.sql"
  run_sql "$rootPath/签到管理.sql"
  run_sql "$rootPath/任务管理.sql"
  run_sql "$rootPath/实名配置.sql"
  run_sql "$rootPath/视频管理.sql"
  run_sql "$rootPath/在线信息.sql"

  rootPath="$workDir/1_4创建存储过程/1_3金币数据库"
  run_sql "$rootPath/标识登录.sql"
  run_sql "$rootPath/查询用户.sql"
  run_sql "$rootPath/兑换管理.sql"
  run_sql "$rootPath/加载机器.sql"
  run_sql "$rootPath/机器配置.sql"
  run_sql "$rootPath/加载配置.sql"
  run_sql "$rootPath/俱乐部管理.sql"
  run_sql "$rootPath/离开房间.sql"
  run_sql "$rootPath/列表描述.sql"
  run_sql "$rootPath/设置权限.sql"
  run_sql "$rootPath/视频服务.sql"
  run_sql "$rootPath/推广管理.sql"
  run_sql "$rootPath/写入费用.sql"
  run_sql "$rootPath/银行服务.sql"
  run_sql "$rootPath/游戏记录.sql"
  run_sql "$rootPath/游戏数据.sql"
  run_sql "$rootPath/游戏写分.sql"

  rootPath="$workDir/1_4创建存储过程/1_5积分数据库"
  run_sql "$rootPath/标识登录.sql"
  run_sql "$rootPath/加载配置.sql"
  run_sql "$rootPath/离开房间.sql"
  run_sql "$rootPath/列表描述.sql"
  run_sql "$rootPath/设置权限.sql"
  run_sql "$rootPath/游戏记录.sql"
  run_sql "$rootPath/游戏写分.sql"

  rootPath="$workDir/1_4创建存储过程/1_6比赛数据库"
  run_sql "$rootPath/比赛管理.sql"
  run_sql "$rootPath/标识登录.sql"
  run_sql "$rootPath/加载配置.sql"
  run_sql "$rootPath/加载消息.sql"
  run_sql "$rootPath/开始结束.sql"
  run_sql "$rootPath/离开房间.sql"
  run_sql "$rootPath/列表描述.sql"
  run_sql "$rootPath/设置权限.sql"
  run_sql "$rootPath/写入费用.sql"
  run_sql "$rootPath/写入奖励.sql"
  run_sql "$rootPath/游戏记录.sql"
  run_sql "$rootPath/游戏写分.sql"

  rootPath="$workDir/1_4创建存储过程/1_7练习数据库"
  run_sql "$rootPath/标识登录.sql"
  run_sql "$rootPath/加载配置.sql"
  run_sql "$rootPath/加载消息.sql"
  run_sql "$rootPath/离开房间.sql"
  run_sql "$rootPath/列表描述.sql"
  run_sql "$rootPath/设置权限.sql"
  run_sql "$rootPath/游戏记录.sql"
  run_sql "$rootPath/游戏写分.sql"

  echo ""
  echo "建立私人房间配置"
  rootPath="$workDir/1_6私人房间"
  run_sql "$rootPath/房间费用.sql"
  run_sql "$rootPath/底分配置.sql"
  run_sql "$rootPath/房间管理.sql"
  run_sql "$rootPath/房间配置.sql"
  run_sql "$rootPath/房卡管理.sql"
  run_sql "$rootPath/房卡信息.sql"
  run_sql "$rootPath/参与信息.sql"
  run_sql "$rootPath/积分写分.sql"
  run_sql "$rootPath/回放信息.sql"
  run_sql "$rootPath/金币写分.sql"

  echo ""
  echo "*************************************************************************"
  echo "              ！！！网狐棋牌精华版一键安装脚本已经执行完毕！！！             "
  echo "                        无需生成游戏标识，请直接关闭                       "
  echo "*************************************************************************"
  pause

  clear || true
  echo ""
  echo "建立游戏标识"
  rootPath="$workDir/1_5创建游戏标识"
  run_sql "$rootPath/标识生成.sql"

  clear || true
  echo "*************************************************************************"
  echo "                                                                         "
  echo "                        ！！！创建游戏标识完毕！！！                        "
  echo "                                                                         "
  echo "                     版权所有： 深圳市网狐科技有限公司                      "
  echo "*************************************************************************"
  pause "按回车退出..."
}

main "$@"
