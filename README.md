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

| 层 | 文件 | 职责 |
|------|------|------|
| **基础类型** ✅ | `log_common.h` | LogLevel、SourceLocation、类型萃取 |
| **日志消息** | `log_msg.h` | 消息结构体（位置 + 级别 + 内容） |
| **单例工厂** | `log_factory.h` | 全局 LogHandle 管理 |
| **核心抽象** | `log_handle.h` | LogSink 管理 + ShouldLog 过滤 |
| **模板层** | `log_variadic_handle.h` | fmt::format 类型安全格式化 |
| **宏系统** | `logger.h` | EXT_LOG_TRACE/DEBUG/INFO/WARN/ERROR/CRITICAL |
| **Sink 层** | `sinks/` | ConsoleSink、FileSink、EffectiveSink |
| **加密** | `crypt/` | AES 加密/解密 |
| **压缩** | `compress/` | zstd / zlib 压缩 |
| **序列化** | `formatter/` | protobuf / 自定义格式化 |
| **异步执行** | `context/` | 线程池 + 异步执行器 |
| **内存映射** | `mmap/` | mmap 映射文件写入 |
| **解码器** | `decode/` | 二进制日志解析/查看 |

## Roadmap

- [x] log_common — 基础类型与日志级别
- [ ] log_msg — 日志消息结构体
- [ ] log_factory — 单例工厂
- [ ] log_handle — 核心 Sink 管理
- [ ] log_variadic_handle — 模板格式化层
- [ ] logger.h — 宏系统
- [ ] sinks — 输出目标（Console/File/Effective）
- [ ] crypt — AES 加密模块
- [ ] compress — zstd/zlib 压缩
- [ ] formatter — protobuf 序列化
- [ ] context — 线程池异步执行
- [ ] mmap — 内存映射写入
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
