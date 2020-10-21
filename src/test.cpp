#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <thread>
#include "gat1400_viid.h"
#include "gat1400_message_factory.h"
#include "json.hpp"

using json = nlohmann::json;

const char* ip = "127.0.0.1";
int port = 8080;
const char* deviceId = "33010101001210000001";
const char* username = "username";
const char* password = "password";

/**
 * �ϴ���Ϣ������һ���ӳ�ʱ�䣨����5s����δ���յ���Ӧ������Ϊ�ϴ�ʧ�ܣ�Ӧ�ش���
 * ��������Σ�����3�Σ��ش���δ�ɹ�������Ϊ��·�������⣬������Ϣ�������½���ע�ᡣ
 * ��ע��ɹ��󣬲�������δ��������
 */
int timeoutMs = 5000; // ��Ӧ���ճ�ʱʱ��(����)
int maxRetries = 3; // ����������

void func_register(security_viid& viid)
{
    int ret;
    while (ret = viid.sys_register(ip, port, deviceId, username, password) != 0) {
        srand((unsigned)time(NULL));
        // Ҫȡ��[a, b)�����������ʹ��(rand() % (b - a)) + a;
        // Ҫȡ��[a, b]�����������ʹ��(rand() % (b - a + 1)) + a;
        // Ҫȡ��(a, b]�����������ʹ��(rand() % (b - a)) + a + 1;
        int interval = (rand() % 300) + 1; // ��
        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }
}

void func_keepalive(security_viid& viid)
{
    int ret;
    while (true) {
        ret = viid.sys_keepalive();
        if (ret < 0) {
            break;
        }
        else if (ret == 0) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

int main(int argc, char* argv[]) {
    int ret;
    security_viid viid;

    security_message_factory msgFactory;
    SecurityString str = msgFactory.makeRegisterMessage("1");

    char* body = (char*)
        "{ \
        \"ResponseStatusObject\": { \
        \"RequestURL\": \"http://localhost:8080/VIID/Register\", \
        \"StatusCode\" : 0, \
        \"StatusString\" : \"ok\", \
        \"Id\" : \"33010299011190000253\", \
        \"LocalTime\" : \"20171220204451\" \
        } \
    }";
    auto j = json::parse(body);
    security_response_status_t status;
    ret = msgFactory.parseResponseStatus(body, status);
    if (ret == 0) {
        printf("ok\n");
    }

    /*{
        "ResponseStatusListObject": {
            "ResponseStatusObject": [
              {
                "RequestURL": "http://localhost:8080/VIID/Images",
                    "StatusCode" : 0,
                    "StatusString" : "����",
                    "Id" : "33010101001210000001022017093009520000031",
                    "LocalTime" : "20171220204451"
            }
            ]
        }
    }*/

    body = (char*)
        "{\"ResponseStatusListObject\": { \
        \"ResponseStatusObject\": [{ \
        \"requestURL\": \"http://localhost:8080/VIID/Images\", \
        \"statusCode\" : 0, \
        \"statusString\" : \"ok\", \
        \"id\" : \"33010299011190000253\", \
        \"localTime\" : \"20171220204451\" \
        } \
        ]} \
    }";
    security_response_statuss_t statuss;
    ret = msgFactory.parseResponseStatusList(body, statuss);
    if (ret == 0) {
        printf("ok\n");
    }

    security_system_time_t time;
    ret = msgFactory.parseSystemTime(body, time);
    if (ret == 0) {
        printf("ok\n");
    }

    ////////////////////////////////////////////////////////////////////////////////////
    std::thread t_register(func_register, std::ref(viid));
    while (!viid.is_registered()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::thread t_keepalive(func_keepalive, std::ref(viid));
    ////////////////////////////////////////////////////////////////////////////////////

    return 0;
}