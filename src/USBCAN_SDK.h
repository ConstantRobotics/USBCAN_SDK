#ifndef USBCAN_SDK_H
#define USBCAN_SDK_H

#include <iostream>
#include <string.h>
#include <vector>
#include <queue>
#include <map>

#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>

#if (defined _WIN32 && defined RF62X_LIBRARY)
#define API_EXPORT __declspec(dllexport)
#else
#define API_EXPORT
#endif

namespace USBCAN_SDK {

enum class Status
{
    OK = 1,
    ERR = 0
};

enum class SendType
{
    NORMAL_SEND = 0,
    SINGLE_SEND = 1
};

enum class RemoteFlag
{
    DATA_FRAME = 0,
    REMOTE_FRAME = 1
};

enum class ExternFlag
{
    STD_FRAME = 0,
    EXT_FRAME = 1
};

enum class RefType
{
    SET_BAUDRATE_CMD = 0,
    SET_FILTER_CMD = 1,
    START_FILTER_CMD = 2,
    CLEAR_FILTER_CMD = 3,
    SET_OVER_TIME = 4,
    SET_AUTO_SEND = 5,
    CLEAR_AUTO_SEND = 6
};

enum class TunnelType
{
    USBCAN_II_TYPE = 4,
    USBCAN_2E_U_TYPE = 21
};

#define FRAME_LEN 8

API_EXPORT class CanTunnel
{
public:
    CanTunnel(void* can_dev, int can_index);
    ~CanTunnel();

    virtual Status init_can();
    Status set_recv_id_list(std::vector<uint32_t> recv_id_list);
    Status clear_buffer();
    Status start_can();
    uint32_t get_receive_num();
    Status reset_can();
    std::vector<std::vector<uint8_t>> recv_data(int length, int wait_time = 400);
    int send_data(int can_id, std::vector<uint8_t> data);
    int read_err_info();

    void push_data_to_recv_queue(std::vector<uint8_t> data);
    std::vector<uint8_t> pop_data_from_recv_queue();

    std::map<std::string, std::string> recv_filter;
    std::map<std::string, std::string> recv_queue;
    std::map<std::string, int> recv_queue_canid;
    std::map<std::string, int> recv_num;

protected:
    void* _dev;
    std::string _can_name;
    int _dev_type;
    int _dev_index;
    int _can_index;

    std::vector<uint8_t> _read_content;



    std::queue<std::vector<uint8_t>> _recv_queue_handle;


    bool _is_running;

    std::vector<uint32_t> _recv_id_list;

};

API_EXPORT class USBCAN_II : public CanTunnel
{
public:
    USBCAN_II(void* can_dev, int can_index);
    ~USBCAN_II();

    Status init_can();
    void* get_init_cfg_by_id();
};

API_EXPORT class CanDev
{
public:
    CanDev(std::string pdcan_name, TunnelType dev_type, int dev_index, int can_index);
    ~CanDev();

    bool is_open();
    Status close();

    std::string get_name();
    int get_dev_type();
    int get_dev_index();
    CanTunnel *get_tunnel();

private:
    USBCAN_II* get_can_dev(int dev_type, int can_index);
    std::string _name;
    int _dev_type;
    int _dev_index;
    int _can_index;
    bool _is_open;
    CanTunnel* _tunnel;
};

API_EXPORT class RecvCanData
{
public:
    RecvCanData(CanTunnel*);
    ~RecvCanData();

    void start();
    void stop();

private:
    void run();
    std::thread _thread;
    bool _stopped = false;
    CanTunnel* _dev;
};


API_EXPORT class CANConnection
{
public:
    CANConnection(
            int send_id, int recv_id, std::string can_name = "GC_USBCAN",
            USBCAN_SDK::TunnelType tunnel_type = USBCAN_SDK::TunnelType::USBCAN_II_TYPE,
            int tunnel_id = 0, int can_index = 1);

    ~CANConnection();

    bool get_connection_status();

    void set_send_id(int send_id);
    std::vector<std::string> pop_recv_cmd(std::string key);
    std::vector<std::string> get_recv_cmd(std::string key);
    int send_cmd(std::vector<uint8_t> cmd);

    USBCAN_SDK::CanTunnel* get_tunnel();

private:
    bool _is_connected;
    USBCAN_SDK::Status init_can();
    USBCAN_SDK::Status start_can();
    void listen_thread();
    void stop();
    USBCAN_SDK::CanDev* _device;
    USBCAN_SDK::CanTunnel* _tunnel;
    void* _pack_thread;
    int _send_id;
    bool _stopped;

    USBCAN_SDK::RecvCanData* _recv_thread;
};

}

#endif // USBCAN_SDK_H
