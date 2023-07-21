

#include<vector>
#include<string.h>
#include<arpa/inet.h>
#include"rocket/common/util.h"
#include"rocket/common/log.h"
#include"rocket/net/coder/tinypb_coder.h"
#include"rocket/net/coder/tinypb_protocol.h"

namespace rocket{

    //将message对象转化为字节流，写入到buffer
    void TinyPBCoder::encode(std::vector<AbstractProtocol::s_ptr> &messages,TcpBuffer::s_ptr out_buffer){
        for(auto &i:messages){
            std::shared_ptr<TinyPBProtocol> msg = std::dynamic_pointer_cast<TinyPBProtocol>(i);
            int len=0;
            const char* buf =encodeTinyPB(msg,len);
            if(buf !=NULL &&len!=0){
                out_buffer->writeToBuffer(buf,len);
            }

            if(buf){
                free((void*)buf);
                buf = NULL;
            }
        }

        
    }

    //将buffer 里面的字节流转换为message 对象
    void TinyPBCoder::decode(std::vector<AbstractProtocol::s_ptr> &out_messages,TcpBuffer::s_ptr buffer){

        while(1){
            //遍历buffer，找到开始符，然后解析出整包的长度，然后得到结束符，判断是否为PB_END
            std::vector<char> tmp =  buffer->m_buffer;
            int start_index =buffer->readIndex();
            int end_index =-1;

            int pk_len = 0;
            bool parse_success =false;
            int i=0;
            for(int i= start_index;i<buffer->writeIndex();++i){
                if(tmp[i] == TinyPBProtocol::PB_START){
                    if(i+1 < buffer->writeIndex()){
                        pk_len = getInt32FromNetBytes(&tmp[i+1]);
                        DEBUGLOG("get pk_len =%d",pk_len);

                        int j=i + pk_len -1;
                        if(j >= buffer->writeIndex()){
                            continue;
                        }
                        if(tmp[j] == TinyPBProtocol::PB_END){
                            start_index = i;
                            end_index = j;
                            parse_success =true;
                            break;
                        }
                    }
                }
            }

            //循环退出条件
            if(i>= buffer->writeIndex()){
                DEBUGLOG("decode end, read all buffer data");
                return ;
            }

            //找到开始结束符，开始解析
            if(parse_success){
                buffer->moveReadIndex(end_index - start_index +1);
                std::shared_ptr<TinyPBProtocol> message =std::make_shared<TinyPBProtocol>();
                message->m_pk_len =pk_len;

                //请求解析
                int msg_id_len_index  = start_index + sizeof(char) + sizeof(message->m_pk_len);
                if(msg_id_len_index >= end_index){
                    message->parse_success =false;
                    ERRORLOG("parse error, msg_id_len_index[%d]>=end_index[%d]",msg_id_len_index,end_index);
                }
                message->m_msg_id_len = getInt32FromNetBytes(&tmp[msg_id_len_index]);
                DEBUGLOG("parse msg_id_len=%d",message->m_msg_id_len);

                int msg_id_index =msg_id_len_index + sizeof(message->m_msg_id_len);

                char msg_id[100] ={0};
                memcpy(&msg_id[0],&tmp[msg_id_index],message->m_msg_id_len);
                message->m_msg_id = std::string(msg_id);
                DEBUGLOG("parse msg_id =%s",message->m_msg_id.c_str());

                //方法解析
                int method_name_len_index = msg_id_index+ message->m_msg_id_len;
                if(method_name_len_index >=end_index){
                    message->parse_success =false;
                    ERRORLOG("parse error, method_name_len_index[%d]>=end_index[%d]",method_name_len_index,end_index);
                }
                message->m_method_name_len = getInt32FromNetBytes(&tmp[method_name_len_index]);

                int method_name_index = method_name_len_index+ sizeof(message->m_method_name_len);
                char method_name[512] ={0};
                memcpy(&method_name[0],&tmp[method_name_index],message->m_method_name_len);
                message->m_method_name = std::string(method_name);
                DEBUGLOG("parse m_method_name =%s",message->m_method_name.c_str());

                //错误解析
                int err_code_index = method_name_index+ message->m_method_name_len;
                if(err_code_index >=end_index){
                    message->parse_success =false;
                    ERRORLOG("parse error, err_code_index[%d]>=end_index[%d]",err_code_index,end_index);
                }
                message->m_err_code = getInt32FromNetBytes(&tmp[err_code_index]);

                int err_info_len_index = err_code_index+ sizeof(message->m_err_code);
                if(err_info_len_index >=end_index){
                    message->parse_success =false;
                    ERRORLOG("parse error, err_info_len_index[%d]>=end_index[%d]",err_info_len_index,end_index);
                }
                message->m_err_info_len = getInt32FromNetBytes(&tmp[err_info_len_index]);

                int err_info_index = err_info_len_index+ sizeof(message->m_err_info_len);
                char err_info[512] ={0};
                memcpy(&err_info[0],&tmp[err_info_index],message->m_err_info_len);
                message->m_err_info = std::string(err_info);
                DEBUGLOG("parse err_info =%s",message->m_err_info.c_str());

                //protobuf序列化数据解析
                int pb_data_len =message->m_pk_len - message->m_method_name_len - message->m_msg_id_len - message->m_err_info_len -2 -24;

                int pb_data_index =err_info_index + message->m_err_info_len;
                message->m_pb_data = std::string(&tmp[pb_data_index],pb_data_len);

                //这里去校验和去解析
                message->parse_success =true;
                out_messages.push_back(message);

            }
        }
        
    }


    const char* TinyPBCoder::encodeTinyPB(std::shared_ptr<TinyPBProtocol> message,int& len){
        if (message->m_msg_id.empty()) {
            message->m_msg_id = "123456789";
        }
        DEBUGLOG("msg_id = %s", message->m_msg_id.c_str());
        int pk_len = 2 + 24 + message->m_msg_id.length() + message->m_method_name.length() + message->m_err_info.length() + message->m_pb_data.length();
        DEBUGLOG("pk_len = %", pk_len);

        char* buf = reinterpret_cast<char*>(malloc(pk_len));
        char* tmp = buf;

        *tmp = TinyPBProtocol::PB_START;
        tmp++;

        int32_t pk_len_net = htonl(pk_len);
        memcpy(tmp, &pk_len_net, sizeof(pk_len_net));
        tmp += sizeof(pk_len_net);

        int msg_id_len = message->m_msg_id.length();
        int32_t msg_id_len_net = htonl(msg_id_len);
        memcpy(tmp, &msg_id_len_net, sizeof(msg_id_len_net));
        tmp += sizeof(msg_id_len_net);

        if (!message->m_msg_id.empty()) {
            memcpy(tmp, &(message->m_msg_id[0]), msg_id_len);
            tmp += msg_id_len;
        }

        int method_name_len = message->m_method_name.length();
        int32_t method_name_len_net = htonl(method_name_len);
        memcpy(tmp, &method_name_len_net, sizeof(method_name_len_net));
        tmp += sizeof(method_name_len_net);

        if (!message->m_method_name.empty()) {
            memcpy(tmp, &(message->m_method_name[0]), method_name_len);
            tmp += method_name_len;
        }

        int32_t err_code_net = htonl(message->m_err_code);
        memcpy(tmp, &err_code_net, sizeof(err_code_net));
        tmp += sizeof(err_code_net);

        int err_info_len = message->m_err_info.length();
        int32_t err_info_len_net = htonl(err_info_len);
        memcpy(tmp, &err_info_len_net, sizeof(err_info_len_net));
        tmp += sizeof(err_info_len_net);

        if (!message->m_err_info.empty()) {
            memcpy(tmp, &(message->m_err_info[0]), err_info_len);
            tmp += err_info_len;
        }

        if (!message->m_pb_data.empty()) {
            memcpy(tmp, &(message->m_pb_data[0]), message->m_pb_data.length());
            tmp += message->m_pb_data.length();
        }

        int32_t check_sum_net = htonl(1);
        memcpy(tmp, &check_sum_net, sizeof(check_sum_net));
        tmp += sizeof(check_sum_net);

        *tmp = TinyPBProtocol::PB_END;

        message->m_pk_len = pk_len;
        message->m_msg_id_len = msg_id_len;
        message->m_method_name_len = method_name_len;
        message->m_err_info_len = err_info_len;
        message->parse_success = true;
        len = pk_len;

        DEBUGLOG("encode message[%s] success", message->m_msg_id.c_str());

        return buf;
    }

}