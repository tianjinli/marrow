#include "WHCommandLine.h"

//////////////////////////////////////////////////////////////////////////////////

// 查询命令
bool CWHCommandLine::SearchCommandItem(LPCTSTR pszCommandLine, LPCTSTR pszCommand, TCHAR szParameter[], WORD wParameterLen) {
  // 效验参数
  ASSERT((pszCommandLine != nullptr) && (pszCommand != nullptr));
  if ((pszCommandLine == nullptr) || (pszCommand == nullptr))
    return false;

  // 参数处理
  if (pszCommandLine[0] != 0) {
    // 变量定义
    UINT nCommandLen = (UINT) StrLenT(pszCommand);
    LPCTSTR lpszBeginString = pszCommandLine;

    // 提取参数
    while (true) {
      // 参数分析
      LPCTSTR lpszEndString = StrChrT(lpszBeginString, TEXT(' '));
      UINT nStringLength = (lpszEndString == nullptr) ? (UINT) StrLenT(lpszBeginString) : (UINT)(lpszEndString - lpszBeginString);

      // 命令分析
      if ((nStringLength >= nCommandLen) && (memcmp(lpszBeginString, pszCommand, nCommandLen * sizeof(TCHAR)) == 0)) {
        // 长度效验
        ASSERT(wParameterLen > (nStringLength - nCommandLen));
        if ((wParameterLen <= (nStringLength - nCommandLen)))
          return false;

        // 提取参数
        szParameter[nStringLength - nCommandLen] = 0;
        CopyMemory(szParameter, lpszBeginString + nCommandLen, (nStringLength - nCommandLen) * sizeof(TCHAR));

        return true;
      }

      // 设置变量
      if (lpszEndString == nullptr)
        break;
      lpszBeginString = (lpszEndString + 1);
    }
  }

  return false;
}

//////////////////////////////////////////////////////////////////////////////////
