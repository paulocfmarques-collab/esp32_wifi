#pragma once
#include <WebServer.h>
#include <lwip/sockets.h>
#include <errno.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Mantem parser e middlewares do core, com inatividade de leitura curta e
// envio normal limitado. Destinado aos handlers String deste projeto, sem SSE.
class BoundedWebServer : public WebServer
{
    static constexpr uint32_t FIRST_BYTE_MS = 1500, READ_IDLE_MS = 250, RESPONSE_MS = 1000;
    bool writing = false;
    uint32_t responseAt = 0;
    uint32_t rejected = 0, aborted = 0;

    void closeClient()
    {
        _currentClient.stop();
        _currentClient = NetworkClient();
        _currentStatus = HC_NONE;
        _currentUpload.reset();
        _currentRaw.reset();
    }

    size_t writeBounded(const char *data, size_t length)
    {
        if (!writing) { writing = true; responseAt = millis(); }
        size_t sent = 0;
        while (sent < length && _currentClient.fd() >= 0 && millis() - responseAt < RESPONSE_MS) {
            const size_t chunk = min(size_t(1024), length - sent);
            const int n = ::send(_currentClient.fd(), data + sent, chunk, MSG_DONTWAIT);
            if (n > 0) sent += size_t(n);
            else if (n == 0 || (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)) break;
            // Garante oportunidade para as tarefas idle durante backpressure TCP.
            if (sent < length) vTaskDelay(1);
        }
        if (sent != length && _currentClient.fd() >= 0) {
            ++aborted;
            _currentClient.stop();
        }
        return sent;
    }

protected:
    size_t _currentClientWrite(const char *data, size_t length) override {
        return writeBounded(data, length);
    }
    size_t _currentClientWrite_P(PGM_P data, size_t length) override {
        return writeBounded(data, length);
    }

public:
    explicit BoundedWebServer(uint16_t port) : WebServer(port) {}
    uint32_t rejectedRequests() const { return rejected; }
    uint32_t abortedResponses() const { return aborted; }

    void handleClient() override
    {
        if (_currentStatus == HC_NONE) {
            _currentClient = _server.accept();
            if (_currentClient.fd() < 0) return;
            _currentStatus = HC_WAIT_READ;
            _statusChange = millis();
        }
        if (_currentStatus != HC_WAIT_READ) {
            // Nenhuma rota do projeto usa streaming ou SSE.
            ++rejected;
            closeClient();
            return;
        }
        if (!_currentClient.connected()) { closeClient(); return; }
        if (!_currentClient.available()) {
            if (millis() - _statusChange >= FIRST_BYTE_MS) {
                ++rejected;
                closeClient();
            }
            return;
        }
        writing = false;
        _currentClient.setTimeout(READ_IDLE_MS);
        // Nao chamar o handleClient base: ele substituiria o timeout por 5000 ms.
        if (_parseRequest(_currentClient)) {
            _contentLength = CONTENT_LENGTH_NOT_SET;
            _responseCode = 0;
            _clearResponseHeaders();
            if (_chain) _chain->runChain(*this, [this]() { return _handleRequest(); });
            else _handleRequest();
        } else ++rejected;
        closeClient();
    }
};
