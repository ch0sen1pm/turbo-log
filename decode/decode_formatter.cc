#include "decode_formatter.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "proto/effective_msg.pb.h"

class FlagFormatter {
public:
    virtual ~FlagFormatter() = default;

    virtual void Format(const EffectiveMsg& flag, std::string& dest) = 0;
};

static void CompilePattern(const std::string& pattern);
static void HandleFlag(char flag);

void DecodeFormatter::SetPattern(const std::string& pattern) {
    CompilePattern(pattern);
}

std::vector<std::unique_ptr<FlagFormatter>> flag_formatters_;

static const char* GetLevelStr(int32_t level) {
    switch (level) {
        case 0: return "V";
        case 1: return "D";
        case 2: return "I";
        case 3: return "W";
        case 4: return "E";
        case 5: return "F";
        default: return "U";
    }
}

std::string CombineLogMsg(const EffectiveMsg& msg) {
    std::string output;
    char buffer[1024] = {0};
    snprintf(buffer, sizeof(buffer), "[%d][%lld][%d:%d][%s:%s:%d]",
             msg.level(), msg.timestamp(), msg.pid(), msg.tid(),
             msg.file_name().c_str(), msg.func_name().c_str(), msg.line());
    output.append(buffer);
    output.append(msg.log_info());
    return output;
}

void DecodeFormatter::Format(const EffectiveMsg& msg, std::string& dest) {
    if (!flag_formatters_.empty()) {
        for (auto& formatter : flag_formatters_) {
            formatter->Format(msg, dest);
        }
    } else {
        dest.append(CombineLogMsg(msg));
    }
    dest.append("\n");
}

class AggregateFormatter final : public FlagFormatter {
public:
    AggregateFormatter() = default;
    ~AggregateFormatter() override = default;

    void AddCh(char ch) { str_ += ch; }
    void Format(const EffectiveMsg& msg, std::string& dest) override { 
        dest.append(str_); 
    }

private:
    std::string str_;
};

class LogLevelFormatter final : public FlagFormatter {
public:
    LogLevelFormatter() = default;
    ~LogLevelFormatter() override = default;

    void Format(const EffectiveMsg& msg, std::string &dest) override { 
        dest.append(GetLevelStr(msg.level())); 
    }
};

std::string MillisecondsToDateString(long long milliseconds) {
    std::chrono::system_clock::time_point tp = 
        std::chrono::system_clock::time_point(std::chrono::milliseconds(milliseconds));
    std::time_t time_tt = std::chrono::system_clock::to_time_t(tp);
    std::tm* timeinfo = std::localtime(&time_tt);
    std::ostringstream oss;
    oss << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

class TimestampDateFormatter final : public FlagFormatter {
public:
    TimestampDateFormatter() = default;
    ~TimestampDateFormatter() override = default;

    void Format(const EffectiveMsg& msg, std::string& dest) override {
        dest.append(MillisecondsToDateString(msg.timestamp()));
    }
};

class TimestampSecondsFormatter final : public FlagFormatter {
public:
    TimestampSecondsFormatter() = default;
    ~TimestampSecondsFormatter() override = default;

    void Format(const EffectiveMsg& msg, std::string& dest) override {
        dest.append(std::to_string(msg.timestamp() / 1000));
    }
};

class TimestampMillisecondsFormatter final : public FlagFormatter {
public:
    TimestampMillisecondsFormatter() = default;
    ~TimestampMillisecondsFormatter() override = default;

    void Format(const EffectiveMsg& msg, std::string& dest) override {
        dest.append(std::to_string(msg.timestamp()));
    }
};

class ProcessIdFormatter final : public FlagFormatter {
public:
    ProcessIdFormatter() = default;
    ~ProcessIdFormatter() override = default;

    void Format(const EffectiveMsg& msg, std::string& dest) override {
        dest.append(std::to_string(msg.pid()));
    }
};

class ThreadIdFormatter final : public FlagFormatter {
public:
    ThreadIdFormatter() = default;
    ~ThreadIdFormatter() override = default;

    void Format(const EffectiveMsg& msg, std::string& dest) override {
        dest.append(std::to_string(msg.tid()));
    }
};

class LineFormatter final : public FlagFormatter {
public:
    LineFormatter() = default;
    ~LineFormatter() override = default;

    void Format(const EffectiveMsg& msg, std::string& dest) override {
        dest.append(std::to_string(msg.line()));
    }
};


class FileNameFormatter final : public FlagFormatter {
public:
    FileNameFormatter() = default;
    ~FileNameFormatter() override = default;

    void Format(const EffectiveMsg& msg, std::string& dest) override{
        dest.append(msg.file_name());
    }
};


class FuncNameFormatter final : public FlagFormatter {
public:
    FuncNameFormatter() = default;
    ~FuncNameFormatter() override = default;

    void Format(const EffectiveMsg& msg, std::string& dest) override {
        dest.append(msg.func_name());
    }
};


class LogInfoFormatter final : public FlagFormatter {
public:
    LogInfoFormatter() = default;
    ~LogInfoFormatter() override = default;

    void Format(const EffectiveMsg& msg, std::string& dest) override {
        dest.append(msg.log_info());
    }
};

void CompilePattern(const std::string& pattern) {
    auto end = pattern.end();
    std::unique_ptr<AggregateFormatter> user_chars;
    flag_formatters_.clear();

    for (auto it = pattern.begin(); it != end; it ++) {
        if (*it == '%') {
            if (user_chars) {
                flag_formatters_.push_back(std::move(user_chars));
            }
            it ++;
            if (it == end) {
                break;
            }
            HandleFlag(*it);
        } else {
            if (!user_chars) {
                user_chars = std::make_unique<AggregateFormatter>();
            }
            user_chars->AddCh(*it);
        }
    }

    if (user_chars) {
        flag_formatters_.push_back(std::move(user_chars));
    }

}

void HandleFlag(char flag) {
    switch (flag) {
        case 'l': flag_formatters_.push_back(std::make_unique<LogLevelFormatter>()); break;
        case 'D': flag_formatters_.push_back(std::make_unique<TimestampDateFormatter>()); break;
        case 'S': flag_formatters_.push_back(std::make_unique<TimestampSecondsFormatter>()); break;
        case 'M': flag_formatters_.push_back(std::make_unique<TimestampMillisecondsFormatter>()); break;
        case 'p': flag_formatters_.push_back(std::make_unique<ProcessIdFormatter>()); break;
        case 't': flag_formatters_.push_back(std::make_unique<ThreadIdFormatter>()); break;
        case '#': flag_formatters_.push_back(std::make_unique<LineFormatter>()); break;
        case 'F': flag_formatters_.push_back(std::make_unique<FileNameFormatter>()); break;
        case 'f': flag_formatters_.push_back(std::make_unique<FuncNameFormatter>()); break;
        case 'v': flag_formatters_.push_back(std::make_unique<LogInfoFormatter>()); break;
        default: {
            auto formatter = std::make_unique<AggregateFormatter>();
            formatter->AddCh('%');
            formatter->AddCh(flag);
            flag_formatters_.push_back(std::move(formatter));
            break;
        }
    }
}





