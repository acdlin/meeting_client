# F5_17 — 基于 Qt 的视频会议客户端

一个使用 Qt 5.15 + C++17 开发的局域网视频会议客户端，支持多人音视频通话、文字聊天、会议室创建与加入。采用多线程架构，音视频采集、网络收发、日志写入分别运行在独立线程，保证 UI 流畅。

## 功能特性

- **视频通话**：摄像头采集 → JPEG 压缩 → 网络传输，支持多路远程视频同屏显示
- **音频通话**：PCM 8kHz/16bit 单声道采集与播放，支持音量调节
- **文字聊天**：实时收发文字消息，带气泡式 UI 和头像
- **会议室**：创建房间（服务端分配 6 位编号）或加入已有房间
- **参与者管理**：动态显示/移除参会者，支持关闭单路摄像头
- **日志系统**：独立线程写日志，不阻塞业务线程

## 技术栈

| 项目 | 说明 |
|------|------|
| 语言 | C++17 |
| 框架 | Qt 5.15.2 |
| 编译器 | MSVC 2019 |
| 构建 | qmake |
| 模块 | core gui widgets network multimedia |

## 项目结构

```
f5_17/
├── f5_17.pro              # qmake 工程文件
├── main.cpp               # 程序入口
├── widget.h/cpp/ui        # 主窗口，业务逻辑总控
├── netheader.h/cpp        # 网络协议定义（MESG 结构 + packMessage）
├── blockqueue.h           # 线程安全阻塞队列模板 QUEUE_DATA<T>
├── mytcpsocket.h/cpp      # TCP socket 封装，帧拆包解析
├── writeworker.h/cpp      # 发送线程，定时从 queue_send 取消息写入 socket
├── sendimg.h/cpp          # 视频发送线程，邮箱模式取最新帧并压缩
├── myvideosurface.h/cpp   # QAbstractVideoSurface，摄像头帧回调
├── audioinput.h/cpp       # 音频采集（QAudioInput）
├── audiooutput.h/cpp      # 音频播放（QAudioOutput）
├── videocell.h/cpp        # 单个视频画面格子控件
├── chatmessage.h/cpp      # 聊天气泡控件
├── partner.h/cpp          # 参与者列表项控件
├── logqueue.h/cpp         # 日志队列 + 后台日志写入线程
└── res.qrc                # 资源文件（默认头像）
```

## 架构设计

### 线程模型

```
┌─────────────────────────────────────────────────┐
│                  主线程 (UI)                     │
│  Widget / VideoCell / ChatMessage / Partner      │
│  QCamera / MyVideoSurface / AudioInput           │
└─────────────────────────────────────────────────┘
       │                    │                │
       ▼                    ▼                ▼
┌──────────┐        ┌──────────────┐  ┌────────────┐
│ SendImg  │        │ MyTcpSocket  │  │ LogWriter  │
│ 视频发送  │        │ +WriteWorker │  │ 日志线程    │
│ 线程     │        │ socket线程   │  │            │
└──────────┘        └──────────────┘  └────────────┘
       │                    │
       ▼                    ▼
  queue_send          queue_recv
  (2000 容量)         (500 容量)
```

- **SendImg 线程**：邮箱模式（只保留最新一帧），避免积压；JPEG 质量设为 50 平衡画质与带宽
- **WriteWorker**：30ms 定时器轮询 `queue_send`，批量写入 socket
- **MyTcpSocket**：独立线程接收，按 `$ ... #` 分隔符拆帧，解析后推入 `queue_recv`
- **LogWriter**：独立线程从 `queue_log` 取日志写入 `logs/app.log`
- **队列满时策略**：`queue_send` 超过 50 帧时丢帧并记录日志，防止音视频互相阻塞

### 网络协议

每帧格式（大端序）：

| 字段 | 长度 | 说明 |
|------|------|------|
| `$`  | 1B   | 帧头 |
| type | 2B   | 消息类型 (msgType) |
| ip   | 4B   | 发送方 IPv4 |
| len  | 4B   | data 长度 |
| data | len  | 负载 |
| `#`  | 1B   | 帧尾 |

消息类型 (`msgType`)：
- `IMG_SEND/RECV`：视频帧（data 前 2B 为端口，其余为 JPEG）
- `AUDIO_SEND/RECV`：音频帧（data 前 2B 为端口，其余为 qCompress 压缩的 PCM）
- `TEXT_SEND/RECV`：文字消息（qCompress 压缩）
- `CREATE/JOIN_MEETING`：创建/加入会议室
- `PARTNER_JOIN/JOIN2/EXIT`：参与者加入/退出通知
- `CLOSE_CAMERA`：关闭摄像头通知

## 构建与运行

### 依赖

- Qt 5.15.2（含 multimedia 模块）
- MSVC 2019（或兼容编译器）
- 摄像头、麦克风设备

### 编译

```bash
# 使用 Qt Creator 打开 f5_17.pro 直接编译
# 或命令行：
qmake f5_17.pro
make        # 或 nmake / jom
```

### 运行

1. 启动配套的服务端程序（监听指定端口）
2. 启动本客户端，在顶部输入服务端 IP 和端口，点击"连接"
3. 连接成功后可"创建"或"加入"会议室
4. 进入会议室后可开关摄像头、麦克风、听筒，并收发文字消息

### 日志查看

运行时日志输出到工作目录下的 `logs/app.log`。

## 关键设计说明

- **邮箱模式取帧**：`SendImg::pushLastestImage` 只保存最新一帧，旧帧被覆盖，避免视频采集快于发送导致积压
- **音视频分队列**：共用 `queue_send` 但音频数据量小，视频通过 `queue_send.size() > 50` 主动丢帧，防止视频阻塞音频
- **断线全清理**：网络断开时按"停止设备 → 清空队列 → 重置 UI"顺序清理，避免残留状态
- **线程退出顺序**：先停 SendImg（生产者）→ 停 WriteWorker → 关闭 socket → 退出 socket 线程 → 停 LogWriter
- **跨线程 UI 操作**：网络线程中需要弹窗时使用 `QTimer::singleShot(0, this, ...)` 切回主线程

