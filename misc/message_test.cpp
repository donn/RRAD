#include <iostream>
#include <vector>
#include <string>
#include "JSON.h"
#include "Message.h"
using namespace RRAD;

JSON msg_json = R"(
{
    "senderID": "string",
    "senderPW": "string",
    "receiverID": "string",
    "requestID": 1,
    "request": true,
    "object": {
        "ownerID": "string",
        "unixTimestamp": 1,
        "id": 1,
        "class": "string"
    },
    "operation": {
        "name": "string",
        "data": {}
    }
}
)"_json;

int main (){
    Message msg(msg_json);
    std::vector<uint8> raw = msg.marshall();
    std::string s(raw.begin(), raw.end());
    std::cout << s << '\n';

    Message reassembled_msg = Message::unmarshall(raw);

    std::cout << reassembled_msg.isRequest() << '\n';

    
    return 0;
}