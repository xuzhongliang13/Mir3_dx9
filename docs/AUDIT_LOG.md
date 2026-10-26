# 提交审计日志（AUDIT_LOG）

> 强制规则（用户要求）：任何提交 push 前提交者必须完成自查；push 后对方必须在 24h 内交叉审计并在此登记。渲染核心改动（WHDXGraphic.cpp 函数体 / Interface.cpp RenderMain / GameWnd.cpp ShowGameWnd / MagicWnd.cpp ShowMagicWnd）未经对方审计 OK 不得视为合并完成。
> 格式：`commit号 | 任务 | 提交者 | 自查 | 审计人 | 审计结论(OK/驳回+理由)`；驳回的提交修复后重新登记一行并标注"重审"。

| commit | 任务 | 提交者 | 自查 | 审计人 | 审计结论 |
|---|---|---|---|---|---|
| 2cf27d9 | T74 底栏条/魔法窗口诊断日志 | MainAgent | OK(编译82fail0, 改动3文件均属P0所有权, 日志已按区域条件写入) | Agent B | OK：WHDXGraphic.cpp 仅按 nY>=500 区域门控加 LogDraw 诊断，未改渲染逻辑/分支；GameWnd/MagicWnd 为 ShowGameWnd/ShowMagicWnd 入口日志，仅追加写盘。建议配合 T77 后再实测确认底栏条 |
| 9fb71b9 | T75 知识库+任务分配文档 | MainAgent | OK(纯文档, 无代码) | Agent B | OK：纯文档，文件所有权隔离清晰，AB01-AB04 分配与任务书一致 |
| e5a3265 | T76 协作审计机制 | MainAgent | OK(纯文档, 无代码) | Agent B | OK：新增审计登记表+双账号规则，逻辑闭环 |
| 77a5816 | T77 DrawWithImageForComp/Ex nxt 行前缀 off-by-one 根因修复 | MainAgent | OK | Agent B | OK：按知识库4.2 RLE格式(每行=[前缀W][数据W字])，`nxt=src+*src++` 正确指向行数据结束，原`+1`会把下一行前缀当RLE码→BADCODE→整帧不画，修复正确；立绘/底栏条/窗口内容共用此函数，T77 应同时解决 AB01 立绘根因(待用户实测确认) |
