
#include<math.h>
#include<algorithm>
#include"rocket/net/tcp/tcp_buffer.h"
#include"rocket/common/log.h"



namespace rocket{

    TcpBuffer::TcpBuffer(int size) : m_size(size){
        m_buffer.resize(size);
    }
    TcpBuffer::~TcpBuffer(){

    }
    //可读字节数
    int TcpBuffer::readAble(){
        return m_write_index-m_read_index;
    } 
    //可写字节数
    int TcpBuffer::writeAble(){
        return m_buffer.size()-m_write_index;
    }
        

    int TcpBuffer::readIndex(){
        return m_read_index;

    }
    int TcpBuffer::writeIndex(){
        return m_write_index;
    }

    void TcpBuffer::writeToBuffer(const char* buf,int size){
        if(size>writeAble()){
            //调整buffer的大小
            int new_size =(int)(1.5 *(m_write_index+size));
            resizeBuffer(new_size);
        }
        memcpy(&m_buffer[m_write_index],buf,size);
    }

    void TcpBuffer::readFromBuffer(std::vector<char> & re,int size){
        if(readAble()==0){
            return ;
        }

        int read_size =readAble()>size?size:readAble();

        std::vector<char> tmp(read_size);
        memcpy(&tmp[0],m_buffer[m_read_index],read_size);

        re.swap(tmp);
        m_read_index +=read_size;

    }


    //有问题，可能存在数据丢失
    void TcpBuffer::resizeBuffer(int new_size){
        std::vector<char> tmp(new_size);
        int count = std::min(new_size,readAble());

        memcpy(&tmp[0],m_buffer[m_read_index],count);
        m_buffer.swap(tmp);
        m_read_index = 0;
        m_write_index = m_read_index+count;

    }

}