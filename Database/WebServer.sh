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
printf '\033]0;%s\007' "网狐棋牌荣耀版一键安装"
echo "当前命令行参数：-S $DB_HOST -U $DB_USER -P $DB_PASS -C -i"

sysOS=$(uname -s)
sysArch=$(uname -m)

run_sql() {
  local file=$1
  # -b：遇到错误退出非零状态；-l 超时；-i 输入文件
  sqlcmd -S $DB_HOST -U $DB_USER -P $DB_PASS -C -i "$file"
}

run_sql_with_db() {
  local db=$1
  local file=$2
  sqlcmd -S $DB_HOST -U $DB_USER -P $DB_PASS -d $db -C -i "$file"
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
  echo "          执行网狐棋牌荣耀版数据库一键安装脚本，自动创建初始数据库。          "
  echo "*************************************************************************"
  pause

  workDir="$workDir/WebServer"

  echo ""
  echo "数据库脚本"
  rootPath="$workDir/1.数据库脚本"
  run_sql "$rootPath/1.数据库删除.sql"
  run_sql "$rootPath/1_1_网站库脚本.sql"
  run_sql "$rootPath/1_2_后台库脚本.sql"
  run_sql "$rootPath/2_1_网站库脚本.sql"
  run_sql "$rootPath/2_2_后台库脚本.sql"

  echo ""
  log "数据脚本"
  rootPath="$workDir/2.数据脚本"
  run_sql "$rootPath/充值服务.sql"
  run_sql "$rootPath/后台数据.sql"
  run_sql "$rootPath/实卡类型.sql"
  run_sql "$rootPath/推广数据.sql"
  run_sql "$rootPath/泡点设置.sql"
  run_sql "$rootPath/独立页面.sql"
  run_sql "$rootPath/站点配置.sql"
  run_sql "$rootPath/系统广告.sql"
  run_sql "$rootPath/网站链接.sql"
  run_sql "$rootPath/转盘数据.sql"
  run_sql "$rootPath/会员属性.sql"

  echo ""
  echo "存储过程"
  rootPath="$workDir/3.存储过程/作业脚本"
  run_sql "$rootPath/每日统计(作业).sql"
  run_sql "$rootPath/统计玩家税收(作业).sql"
  run_sql "$rootPath/统计代理充值(作业).sql"

  rootPath="$workDir/3.存储过程/公共过程"
  run_sql_with_db "RYAccountsDB" "$rootPath/分页过程.sql"
  run_sql_with_db "RYGameMatchDB" "$rootPath/分页过程.sql"
  run_sql_with_db "RYGameScoreDB" "$rootPath/分页过程.sql"
  run_sql_with_db "RYNativeWebDB" "$rootPath/分页过程.sql"
  run_sql_with_db "RYPlatformDB" "$rootPath/分页过程.sql"
  run_sql_with_db "RYPlatformManagerDB" "$rootPath/分页过程.sql"
  run_sql_with_db "RYRecordDB" "$rootPath/分页过程.sql"
  run_sql_with_db "RYTreasureDB" "$rootPath/分页过程.sql"

  run_sql_with_db "RYAccountsDB" "$rootPath/切字符串.sql"
  run_sql_with_db "RYGameMatchDB" "$rootPath/切字符串.sql"
  run_sql_with_db "RYGameScoreDB" "$rootPath/切字符串.sql"
  run_sql_with_db "RYNativeWebDB" "$rootPath/切字符串.sql"
  run_sql_with_db "RYPlatformDB" "$rootPath/切字符串.sql"
  run_sql_with_db "RYPlatformManagerDB" "$rootPath/切字符串.sql"
  run_sql_with_db "RYRecordDB" "$rootPath/切字符串.sql"
  run_sql_with_db "RYTreasureDB" "$rootPath/切字符串.sql"

  rootPath="$workDir/3.存储过程/函数"
  run_sql "$rootPath/查询指定玩家的代理玩家.sql"

  rootPath="$workDir/3.存储过程/前台脚本/本地数据库"
  run_sql "$rootPath/推荐游戏.sql"
  run_sql "$rootPath/购买奖品.sql"

  rootPath="$workDir/3.存储过程/前台脚本/比赛数据库"
  run_sql "$rootPath/比赛排行.sql"

  rootPath="$workDir/3.存储过程/前台脚本/用户数据库"
  run_sql "$rootPath/修改密码.sql"
  run_sql "$rootPath/修改资料.sql"
  run_sql "$rootPath/固定机器.sql"
  run_sql "$rootPath/奖牌兑换.sql"
  run_sql "$rootPath/每日签到.sql"
  run_sql "$rootPath/用户全局信息.sql"
  run_sql "$rootPath/用户名检测.sql"
  run_sql "$rootPath/用户注册.sql"
  run_sql "$rootPath/用户登录.sql"
  run_sql "$rootPath/获取用户信息.sql"
  run_sql "$rootPath/账户保护.sql"
  run_sql "$rootPath/重置密码.sql"
  run_sql "$rootPath/魅力兑换.sql"
  run_sql "$rootPath/自定头像.sql"

  rootPath="$workDir/3.存储过程/前台脚本/积分数据库"
  run_sql "$rootPath/负分清零.sql"
  run_sql "$rootPath/逃率清零.sql"

  rootPath="$workDir/3.存储过程/前台脚本/网站数据库"
  run_sql "$rootPath/更新浏览.sql"
  run_sql "$rootPath/比赛报名.sql"
  run_sql "$rootPath/获取新闻.sql"
  run_sql "$rootPath/购买奖品.sql"
  run_sql "$rootPath/问题反馈.sql"

  rootPath="$workDir/3.存储过程/前台脚本/金币数据库"
  run_sql "$rootPath/代理结算.sql"
  run_sql "$rootPath/在线充值.sql"
  run_sql "$rootPath/在线订单.sql"
  run_sql "$rootPath/实卡充值.sql"
  run_sql "$rootPath/推广中心.sql"
  run_sql "$rootPath/推广信息.sql"
  run_sql "$rootPath/苹果充值.sql"
  run_sql "$rootPath/金币取款.sql"
  run_sql "$rootPath/金币存款.sql"
  run_sql "$rootPath/金币转账.sql"
  run_sql "$rootPath/手游充值.sql"
  run_sql "$rootPath/分享赠送.sql"
  run_sql "$rootPath/转盘抽奖.sql"

  rootPath="$workDir/3.存储过程/后台脚本/帐号库"
  run_sql "$rootPath/插入限制IP.sql"
  run_sql "$rootPath/插入限制机器码.sql"
  run_sql "$rootPath/更新用户.sql"
  run_sql "$rootPath/注册IP统计.sql"
  run_sql "$rootPath/注册机器码统计.sql"
  run_sql "$rootPath/添加用户.sql"
  run_sql "$rootPath/创建代理.sql"

  rootPath="$workDir/3.存储过程/后台脚本/平台库"
  run_sql "$rootPath/在线统计.sql"

  rootPath="$workDir/3.存储过程/后台脚本/数据分析"
  run_sql "$rootPath/充值统计.sql"
  run_sql "$rootPath/其他统计.sql"
  run_sql "$rootPath/活跃统计.sql"
  run_sql "$rootPath/用户统计.sql"
  run_sql "$rootPath/金币分布.sql"

  rootPath="$workDir/3.存储过程/后台脚本/权限库"
  run_sql "$rootPath/权限加载.sql"
  run_sql "$rootPath/用户表操作.sql"
  run_sql "$rootPath/管理员登录.sql"
  run_sql "$rootPath/菜单加载.sql"

  rootPath="$workDir/3.存储过程/后台脚本/比赛库"
  run_sql "$rootPath/比赛排名.sql"

  rootPath="$workDir/3.存储过程/后台脚本/积分库"
  run_sql "$rootPath/清零积分.sql"
  run_sql "$rootPath/清零逃率.sql"
  run_sql "$rootPath/赠送积分.sql"

  rootPath="$workDir/3.存储过程/后台脚本/网站库"
  run_sql "$rootPath/删商品类.sql"

  rootPath="$workDir/3.存储过程/后台脚本/记录库"
  run_sql "$rootPath/赠送会员.sql"
  run_sql "$rootPath/赠送经验.sql"
  run_sql "$rootPath/赠送金币.sql"
  run_sql "$rootPath/赠送靓号.sql"

  rootPath="$workDir/3.存储过程/后台脚本/金币库"
  run_sql "$rootPath/代理分成详情.sql"
  run_sql "$rootPath/增删道具.sql"
  run_sql "$rootPath/实卡入库.sql"
  run_sql "$rootPath/实卡统计.sql"
  run_sql "$rootPath/数据汇总.sql"
  run_sql "$rootPath/新增实卡.sql"
  run_sql "$rootPath/游戏记录.sql"
  run_sql "$rootPath/统计记录.sql"
  run_sql "$rootPath/赠送金币.sql"
  run_sql "$rootPath/转账税收.sql"
  run_sql "$rootPath/统计代理充值(手工执行).sql"
  run_sql "$rootPath/统计玩家税收(手工执行).sql"

  rootPath="$workDir/4.创建作业"
  run_sql "$rootPath/创建作业.sql"
  run_sql "$rootPath/代理充值统计.sql"
  run_sql "$rootPath/税收统计.sql"

  clear || true
  echo "*************************************************************************"
  echo "                                                                         "
  echo "                        ！！！数据库已建立完成！！！                        "
  echo "                                                                         "
  echo "                     版权所有： 深圳市网狐科技有限公司                      "
  echo "*************************************************************************"
  pause "按回车退出..."
}

main "$@"
