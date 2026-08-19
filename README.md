# turbo-log

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

C++ 高性能日志系统——深入理解工业级日志的每个环节。

> 从底层 mmap 内存映射写入到上层宏系统，完整掌握高性能日志系统的设计与实现。

## 特性

- **编译期级别过滤** — 禁用的日志级别零开销（`#if` 预处理）
- **双缓冲 mmap 写入** — 绕过内核缓冲区，直接映射到磁盘
- **线程池异步落盘** — 日志调用不阻塞主线程
- **zstd / zlib 压缩** — 高压缩比 + 快速压缩
- **AES 加密** — 敏感日志加密存储
- **protobuf 序列化** — 二进制高效存储（替代纯文本）
- **多 Sink 可插拔** — 控制台 / 文件 / 加密压缩 自由组合
- **格式安全** — `fmt::format_string<>` 编译期检查格式串正确性

## 架构

```
EXT_LOG_INFO("hello {}", 42)           ← 宏层（零开销）
  ↓
LogFactory (单例)                       ← 持有当前 LogHandle
  ↓
VariadicLogHandle                      ← fmt 格式化 + 转发
  ↓
LogHandle → ShouldLog?                 ← 运行时级别过滤
  ↓
LogSink (多态)                         ← 可插拔输出目标
  ├── ConsoleSink                      ← 控制台彩色输出
  ├── FileSink                         ← 普通文件写入
  └── EffectiveSink                    ← 加密 + 压缩 + pb 序列化
       ├── Crypt (AES)                 ← 加密模块
       ├── Compress (zstd/zlib)        ← 压缩模块
       └── Formatter (protobuf)        ← 序列化模块
```

## 模块分层

### 核心链路（已贯通 ✅）

一条日志从调用到输出的完整路径：

| 层 | 文件 | 职责 | 状态 |
|------|------|------|------|
| **基础类型** | `log_common.h` | LogLevel、SourceLocation、类型别名、编译期级别宏 | ✅ |
| **日志消息** | `log_msg.h/.cc` | 消息结构体（位置 + 级别 + 内容） | ✅ |
| **格式化接口** | `formatter/formatter.h` | 抽象基类：Format 接口 | ✅ |
| **Sink 接口** | `sinks/sink.h` | 抽象基类：Log / SetFormatter / Flush | ✅ |
| **核心调度** | `log_handle.h/.cc` | Sink 管理 + ShouldLog 过滤 + atomic 级别 | ✅ |
| **模板层** | `log_variadic_handle.h` | fmt::format 编译期类型安全检查 | ✅ |
| **单例工厂** | `log_factory.h/.cc` | 全局 VariadicLogHandle 管理 | ✅ |
| **宏系统** | `logger.h` | EXT_LOG_TRACE/DEBUG/INFO/WARN/ERROR/CRITICAL + 编译期零开销过滤 | ✅ |

### 实现层

| 层 | 文件 | 职责 | 状态 |
|------|------|------|------|
| **纯文本格式化** | `formatter/default_formatter.h/.cc` | 日期 + 级别 + 位置 + pid:tid + 消息 | ✅ |
| **控制台输出** | `sinks/console_sink.h/.cc` | stdout 输出 | ✅ |
| **系统工具** | `utils/sys_util.h/.cc` | GetPageSize、GetProcessId、LocalTime | ✅ |
| **RAII 工具** | `defer.h` | 作用域退出自动清理 | ✅ |
| **文件工具** | `utils/file_util.h/.cc` | GetFileSize | ✅ |
| **内存映射** | `mmap/mmap_aux.h/.cc` `mmap/mmap_linux.cc` | mmap 双缓冲写入：魔数校验 + 按页扩容 | ✅ |
| **加密** | `crypt/crypt.h/.cc` `crypt/aes_crypt.h/.cc` | ECDH 密钥协商 + AES-256 CBC 加解密 | ✅ |
| **压缩** | `compress/` | 抽象接口 + zstd + zlib 双实现 | ✅ |

### 待完成

| 层 | 文件 | 职责 |
|------|------|------|
| **pb 格式化** | `formatter/effective_formatter.h/.cc` | protobuf 二进制序列化 |
| **高效 Sink** | `sinks/effective_sink.h/.cc` | 加密 + 压缩 + mmap 落盘 |
| **异步执行** | `context/` | 线程池 + 异步执行器 |
| **解码器** | `decode/` | 二进制日志解析/查看 |

## Roadmap

- [x] log_common — 基础类型与日志级别
- [x] log_msg — 日志消息结构体
- [x] formatter — 抽象格式化接口
- [x] sink — 抽象 Sink 接口
- [x] log_handle — 核心 Sink 管理 + ShouldLog 过滤
- [x] log_variadic_handle — 模板格式化层（编译期格式检查）
- [x] log_factory — 单例工厂
- [x] logger.h — 宏系统（编译期零开销级别过滤）
- [x] sys_util — 系统工具（GetPageSize / GetProcessId / LocalTime）
- [x] defer — RAII 作用域退出自动清理
- [x] file_util — 文件大小工具
- [x] mmap — 内存映射文件写入（mmap_aux + mmap_linux）
- [x] default_formatter — 纯文本格式化
- [x] console_sink — 控制台输出
- [x] CMake + example — 构建系统 + 示例（第一条日志已跑通）
- [x] crypt — ECDH 密钥协商 + AES 加密
- [x] compress — zstd/zlib 压缩
- [ ] formatter 实现 — protobuf 序列化
- [ ] context — 线程池异步执行（thread_pool 完整，executor.h 已写，executor.cc/定时器/context 待写）
- [ ] effective_sink — 加密 + 压缩 + mmap 落盘
- [ ] decode — 二进制日志解码器

## Quick Start

```bash
# 依赖
# zstd-1.5.6, zlib-1.2.13, protobuf-21.8, cryptopp-8.9.0, fmt-11.0.2

# 编译（需先配置 vcpkg/cmake 依赖）
cmake -B build
cmake --build build

# 示例
LOG_INFO("hello {}!", "world");
```

## License

MIT
