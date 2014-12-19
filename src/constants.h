#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#define ERR_SYNTAX "Syntax error"
#define ERR_CREATE_FILE "Create file faild"
#define ERR_BAD_CONF_FILE "Bad or empty config file"
#define ERR_MULTI_LISTEN_HOST "Can bind to only ONE listen address"
#define ERR_MULTI_STORE "Can specify only ONE store per kids. consider using multiple stor."
#define ERR_SEND_SECONDARY "Send to secondary store failed, message lost"
#define ERR_CREATE_ZMQ_CONTEXT "Create ZeroMQ context faild"
#define ERR_CREATE_DIR "Create dir failed"
#define ERR_READ_FILE "Read file failed"
#define ERR_WRITE_FILE "Write file failed"
#define ERR_OPEN_FILE "Open file failed"
#define ERR_PARSE_CONF "Parse config file error"
#define ERR_BAD_ADDR "Bad address"
#define ERR_NO_LISTEN_ADDR "No listen socket or address specified"

#define ERR_CREATE_STORE "Create store failed"
#define ERR_INIT_STORE "Init store failed"

#define ERR_CREATE_SOCK "Create pull socket faild"
#define ERR_CONNECT "Connect socket faild"
#define ERR_RECV "Recv faild"
#define ERR_SEND "Send faild"
#define ERR_BIND "Bind socket faild"

#define ERR_CONF_UNKOWN_STORE_FIELD "Unkonw store field"
#define ERR_CONF_UNKOWN_KIDS_FIELD "Unkonw kids field"
#define ERR_CONF_AT_LEAST_1 "You must specify at least one store for multiple/priority store"
#define ERR_CONF_BUF_1 "You must specify exactly two store for buffer store"
#define ERR_CONF_BUF_2 "You must tell me if the store is primary or secondary for buffer store"
#define ERR_CONF_UNKOWN_STORE_TYPE "Unkonw store type"

#define REP_OK "+OK\r\n"
#define REP_OK_SIZE (sizeof(REP_OK) - 1)
#define REP_PONG "+PONG\r\n"
#define REP_PONG_SIZE (sizeof(REP_PONG) - 1)
#define REP_CZERO ":0\r\n"
#define REP_CZERO_SIZE (sizeof(REP_CZERO) - 1)
#define REP_CONE ":1\r\n"
#define REP_CONE_SIZE (sizeof(REP_CONE) - 1)
#define REP_MBULK3 "*3\r\n"
#define REP_MBULK3_SIZE (sizeof(REP_MBULK3) - 1)
#define REP_MBULK4 "*4\r\n"
#define REP_MBULK4_SIZE (sizeof(REP_MBULK4) - 1)
#define REP_MESSAGEBULK "$7\r\nmessage\r\n"
#define REP_MESSAGEBULK_SIZE (sizeof(REP_MESSAGEBULK) - 1)
#define REP_PMESSAGEBULK "$8\r\npmessage\r\n"
#define REP_PMESSAGEBULK_SIZE (sizeof(REP_PMESSAGEBULK) - 1)
#define REP_SUBSCRIBEBULK "$9\r\nsubscribe\r\n"
#define REP_SUBSCRIBEBULK_SIZE (sizeof(REP_SUBSCRIBEBULK) - 1)
#define REP_UNSUBSCRIBEBULK "$11\r\nunsubscribe\r\n"
#define REP_UNSUBSCRIBEBULK_SIZE (sizeof(REP_UNSUBSCRIBEBULK) - 1)
#define REP_PSUBSCRIBEBULK "$10\r\npsubscribe\r\n"
#define REP_PSUBSCRIBEBULK_SIZE (sizeof(REP_PSUBSCRIBEBULK) - 1)
#define REP_PUNSUBSCRIBEBULK "$12\r\npunsubscribe\r\n"
#define REP_PUNSUBSCRIBEBULK_SIZE (sizeof(REP_PUNSUBSCRIBEBULK) - 1)

#endif // CONSTANTS_H_
