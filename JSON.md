# Message
## Using library: nlohmann::json
## Class structure in C++
All classes representing objects for use over the network should inherit from a superobject named `RemoteObject`. This doesn't maintain anything except mandate a virtual method.

```c++
virtual JSON executeRPC(std::string name, JSON arguments) = 0;
virtual std::string getClassName() = 0; // For filtering purposes
```

A `Dispatcher` class is used, which has a list of all networked objects in a hashmap. The dispatcher calls executeRPC. It is up to the individual classes to implement executeRPC.

## Marshalled as JSON
```js
{
    "senderID": string,
    "receiverID": string,
    "requestID": number,
    "request": boolean,
    "object": {
        "ownerID": string,
        "unixTimestamp": number,
        "id": number,
        "class": string
    },
    "operation": {
        "name": string,
        "data": Object // Arguments for request, Returned values for reply
    }
}
```

If unixTimestamp = 0 && id = 0: You get a list.

NOTE: Big binary blobs become basically marshalled to be base64.