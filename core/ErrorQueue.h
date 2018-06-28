#ifndef SORBET_ERRORQUEUE_H
#define SORBET_ERRORQUEUE_H
#include "Errors.h"
#include "common/ConcurrentQueue.h"
#include "lsp/QueryResponse.h"
#include "spdlog/spdlog.h"
#include <atomic>
#include <climits> // INT_MAX

namespace spd = spdlog;

namespace sorbet {
namespace core {
// This class has to be in a separate header to make us compile /facepalm:
//  - parser defines a constant TRUE in header generated by bison\regel
//  - ConcurrentBoundedQueue on mac includes some OS headers that "#define TRUE 1"
//

struct ErrorQueueMessage {
    enum class Kind { Error, Flush, QueryResponse };
    Kind kind;
    FileRef whatFile;
    std::string text;
    std::unique_ptr<BasicError> error;
    std::unique_ptr<QueryResponse> queryResponse;
};

struct ErrorQueue {
private:
    std::thread::id owner;
    std::unordered_map<core::FileRef, std::vector<ErrorQueueMessage>> collected;
    ConcurrentUnBoundedQueue<ErrorQueueMessage> queue;

    void renderForFile(core::FileRef whatFile, std::stringstream &critical, std::stringstream &nonCritical);

public:
    spd::logger &logger;
    spd::logger &tracer;
    std::atomic<bool> hadCritical{false};
    std::atomic<int> errorCount{0};

    ErrorQueue(spd::logger &logger, spd::logger &tracer);
    ~ErrorQueue();

    std::vector<std::unique_ptr<QueryResponse>> drainQueryResponses();
    std::vector<std::unique_ptr<BasicError>> drainErrors();
    void push(ErrorQueueMessage msg);
    void flushFile(FileRef file);
    void flushErrors(bool all = false);
};
} // namespace core
} // namespace sorbet

#endif // SORBET_ERRORQUEUE_H
