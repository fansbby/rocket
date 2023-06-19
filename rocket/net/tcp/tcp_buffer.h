#ifndef ROCKET_NET_TCP_BUFFER_H
#define ROCKET_NET_TCP_BUFFER_H

#include<vector>

namespace rocket{

class TcpBuffer{
private:
    /* data */
public:
    TcpBuffer(int size);
    ~TcpBuffer();
    int readAble(); //可读字节数
    int writeAble();//可写字节数

    int readIndex();
    int writeIndex();

    void writeToBuffer(const char* buf,int size);
    void readFromBuffer(std::vector<char> & re,int size);

    void resizeBuffer(int new_size);

private:
    int m_read_index{0};
    int m_write_index{0};
    int m_size{0};
    std::vector<char> m_buffer{NULL};

};

}



#endif