#include "USBCAN_SDK.h"

#ifdef ZLG_USBCAN
#include "ControlCAN.h"
#else
#include "ECanVci.h"
#endif

using namespace USBCAN_SDK;

CanDev::CanDev(std::string pdcan_name, TunnelType dev_type, int dev_index, int can_index)
{
    _name = pdcan_name;
    _dev_type = (int)dev_type;
    _dev_index = dev_index;
    _can_index = can_index;
    _is_open = false;

    Status ret = Status::ERR;
    if (pdcan_name == "GC_USBCAN")
    {
        for (int i = 0; i < 3; i++)
        {
            ret = (Status)OpenDevice(_dev_type, _dev_index, 0);
            if (ret == Status::OK)
            {
                _is_open = true;
                break;
            }
        }

        _tunnel = get_can_dev(_dev_type, _can_index);
    }
}

USBCAN_II* CanDev::get_can_dev(int dev_type, int can_index)
{
    if (_dev_type == dev_type)
    {
        return new USBCAN_II((void*)this, can_index);
    }
    else return nullptr;
}

std::string CanDev::get_name()
{
    return _name;
}

int CanDev::get_dev_type()
{
    return _dev_type;
}

int CanDev::get_dev_index()
{
    return _dev_index;
}

CanTunnel *CanDev::get_tunnel()
{
    return _tunnel;
}

bool CanDev::is_open()
{
    return _is_open;
}

Status CanDev::close()
{
    if (_is_open)
        return (Status)CloseDevice(_dev_type, _dev_index);
    else
        return Status::ERR;
}

CanTunnel::CanTunnel(void *can_dev, int can_index)
{
    _dev = can_dev;
    _dev_type = ((CanDev*)_dev)->get_dev_type();
    _dev_index = ((CanDev*)_dev)->get_dev_index();
    _can_name = ((CanDev*)_dev)->get_name();
    _can_index = can_index;

    _read_content = std::vector<uint8_t>();

    _is_running = false;
    _recv_queue_handle = std::queue<std::vector<uint8_t>>();
    _recv_id_list = std::vector<uint32_t>();

}

Status CanTunnel::init_can()
{
    return Status::OK;
}

Status CanTunnel::set_recv_id_list(std::vector<uint32_t> recv_id_list)
{
    _recv_id_list = recv_id_list;
    return Status::OK;
}

Status CanTunnel::clear_buffer()
{
    return (Status)ClearBuffer(_dev_type, _dev_index, _can_index);
}

Status CanTunnel::start_can()
{
    Status ret = Status::ERR;
    if (((CanDev*)_dev)->is_open())
    {
        ret = (Status)StartCAN(_dev_type, _dev_index, _can_index);
        if (ret == Status::OK)
            _is_running = true;
    }

    return ret;

}

uint32_t CanTunnel::get_receive_num()
{
    if (_is_running)
        return GetReceiveNum(_dev_type, _dev_index, _can_index);
    else
        return 0;
}

Status CanTunnel::reset_can()
{
    if (_is_running)
        return (Status)ResetCAN(_dev_type, _dev_index, _can_index);
    else
        return Status::ERR;
}

std::vector<std::vector<uint8_t>> CanTunnel::recv_data(int length, int wait_time)
{
    if (_is_running)
    {
        CAN_OBJ* recv_buf = new CAN_OBJ[length]();
        int recv_len = 0;
        try
        {
            recv_len = Receive(_dev_type, _dev_index, _can_index, recv_buf, length, wait_time);
            std::vector<std::vector<uint8_t>> data_list = std::vector<std::vector<uint8_t>>();
            for (int i = 0; i < recv_len; i++)
            {
                std::vector<uint8_t> can_data  = std::vector<uint8_t>();
                for (size_t j = 0; j < _recv_id_list.size(); j++)
                {
                    if (recv_buf[i].ID == _recv_id_list[j])
                        for (int n = 0; n < recv_buf[i].DataLen; n++)
                            can_data.push_back(recv_buf[i].Data[n]);
                }
                if (can_data.size() > 0)
                    data_list.push_back(can_data);
            }

            delete[] recv_buf;
            return data_list;
        }
        catch (...)
        {
            delete[] recv_buf;
            return std::vector<std::vector<uint8_t>>();
        }
    }else
    {
        return std::vector<std::vector<uint8_t>>();
    }
}

int CanTunnel::send_data(int can_id, std::vector<uint8_t> data)
{
    if (_is_running)
    {
        int data_len = (int)data.size();
        int frame_num = 0;
        int full_frame_num = data_len / FRAME_LEN;
        int left_len = data_len % FRAME_LEN;

        if (left_len == 0)
            frame_num = full_frame_num;
        else
            frame_num = full_frame_num + 1;

        CAN_OBJ* send_buf = new CAN_OBJ[frame_num]();

        int data_offset = 0;
        for (int i = 0; i < (int)(full_frame_num); i++)
        {
            send_buf[i].ID = can_id;
            send_buf[i].SendType = (uint8_t)SendType::NORMAL_SEND;
            send_buf[i].RemoteFlag = (uint8_t)RemoteFlag::DATA_FRAME;
            send_buf[i].ExternFlag = (uint8_t)ExternFlag::STD_FRAME;
            send_buf[i].DataLen = FRAME_LEN;

            for (int j = 0; j < FRAME_LEN; j++)
            {
                send_buf[i].Data[j] = data[data_offset+j];
            }
            data_offset +=FRAME_LEN;
        }

        if (left_len > 0)
        {
            send_buf[frame_num - 1].ID = can_id;
            send_buf[frame_num - 1].SendType = (uint8_t)SendType::NORMAL_SEND;
            send_buf[frame_num - 1].RemoteFlag = (uint8_t)RemoteFlag::DATA_FRAME;
            send_buf[frame_num - 1].ExternFlag = (uint8_t)ExternFlag::STD_FRAME;
            send_buf[frame_num - 1].DataLen = left_len;

            for (int j = 0; j < left_len; j++)
                send_buf[frame_num - 1].Data[j] = data[data_offset+j];
        }

        int send_len = Transmit(_dev_type, _dev_index, _can_index, send_buf, frame_num);

        if (send_len == frame_num)
            return send_len;
        else
            return 0;
    }else
        return 0;

}

int CanTunnel::read_err_info()
{
    ERR_INFO* err_info = new ERR_INFO();
    Status ret = (Status)ReadErrInfo(_dev_type, _dev_index, _can_index, err_info);
    if (ret == Status::OK)
        return err_info->ErrCode;
    else
    {
        //logger.info("can not get the err_code")
        return 0;
    }
}

void CanTunnel::push_data_to_recv_queue(std::vector<uint8_t> data)
{
    _recv_queue_handle.push(data);
}

std::vector<uint8_t> CanTunnel::pop_data_from_recv_queue()
{
    if (!_recv_queue_handle.empty())
    {
        std::vector<uint8_t> ret = _recv_queue_handle.front();
        _recv_queue_handle.pop();
        return ret;
    }else
    {
        return std::vector<uint8_t>();
    }
}


USBCAN_II::USBCAN_II(void* can_dev, int can_index) : CanTunnel(can_dev, can_index)
{

}

Status USBCAN_II::init_can()
{
    INIT_CONFIG* init_config = (INIT_CONFIG*)get_init_cfg_by_id();
    int a = InitCAN(_dev_type, _dev_index, _can_index, init_config);
    Status status = (Status)a;
    return status;
}

void* USBCAN_II::get_init_cfg_by_id()
{
    //receive all canid
    uint32_t ACR = 0x00000000;
    uint32_t AMR = 0xFFFFFFFF;
    uint32_t reserved = 0;

    //one id filter
    uint8_t filter_byte = 1;

    //can baudrate 1000K
    uint8_t timing0 = 0x00;
    uint8_t timing1 = 0x14;

    //normal mode
    uint8_t mode = 0;

    INIT_CONFIG* init_config = (INIT_CONFIG*)calloc(1, sizeof (INIT_CONFIG));
    init_config->AccCode = ACR;
    init_config->AccMask = AMR;
    init_config->Reserved = reserved;
    init_config->Filter = filter_byte;
    init_config->Timing0 = timing0;
    init_config->Timing1 = timing1;
    init_config->Mode = mode;

    return init_config;

}

RecvCanData::RecvCanData(CanTunnel *can_tunnel)
{
    _stopped = false;
    _dev = can_tunnel;
}

void RecvCanData::start()
{
    _thread = std::thread(&RecvCanData::run, this);
}

void RecvCanData::run()
{
    while (!_stopped)
    {
        uint32_t num = _dev->get_receive_num();
        if (num > 0)
        {
            std::vector<std::vector<uint8_t>> recv_data = _dev->recv_data(num);
            for(size_t i = 0; i < recv_data.size(); i++)
                ((CanTunnel*)_dev)->push_data_to_recv_queue(recv_data[i]);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void RecvCanData::stop()
{
    _stopped = true;
    _thread.join();
}



CANConnection::CANConnection(
        int send_id, int recv_id, std::string can_name,
        TunnelType tunnel_type, int tunnel_id, int can_index)
{
    _is_connected = false;
    _device = new CanDev(can_name, tunnel_type, tunnel_id, can_index);
    _tunnel = _device->get_tunnel();

    std::vector<uint32_t> id_list = std::vector<uint32_t>();
    id_list.push_back(recv_id);
    _tunnel->set_recv_id_list(id_list);
    set_send_id(send_id);

    _stopped = false;

    Status ret = Status::ERR;
    ret = init_can();
    if (ret == Status::OK)
    {
        printf("init can\n");
        ret = start_can();
        if (ret == Status::OK)
        {
            printf("start can\n");

            listen_thread();
            printf("listen thread\n");
        }
    }

    if (ret == Status::OK)
        _is_connected = true;
    else
        _is_connected = false;
}

bool CANConnection::get_connection_status()
{
    return _is_connected;
}

void CANConnection::set_send_id(int send_id)
{
    _send_id = send_id;
}

std::vector<std::string> CANConnection::pop_recv_cmd(std::string key)
{
    std::vector<std::string> recv_cmd = std::vector<std::string>();
    for (auto itr = _tunnel->recv_queue.begin(); itr != _tunnel->recv_queue.end(); ++itr)
    {
        if (itr->first == key)
        {
            recv_cmd.push_back(itr->second);
            _tunnel->recv_queue.erase(itr);
        }
    }

    return recv_cmd;
}

std::vector<std::string> CANConnection::get_recv_cmd(std::string key)
{
    std::vector<std::string> recv_cmd = std::vector<std::string>();
    for (auto itr = _tunnel->recv_queue.begin(); itr != _tunnel->recv_queue.end(); ++itr)
    {
        if (itr->first == key)
        {
            recv_cmd.push_back(itr->second);
        }
    }

    return recv_cmd;
}

int CANConnection::send_cmd(std::vector<uint8_t> cmd)
{
    return _tunnel->send_data(_send_id, cmd);
}

CanTunnel *CANConnection::get_tunnel()
{
    return _tunnel;
}

Status CANConnection::init_can()
{
    return _tunnel->init_can();
}

Status CANConnection::start_can()
{
    _tunnel->clear_buffer();
    return _tunnel->start_can();
}

void CANConnection::listen_thread()
{
    _recv_thread = new RecvCanData(_tunnel);
    _recv_thread->start();
}

void CANConnection::stop()
{
    _stopped = true;
}
