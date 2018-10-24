# Message
## Using library: nlohmann::json

## Assumptions
* Request port is 9000 for peers.
    * ADS Request port is 10000 (for now).
* ADS IP is provided with the terminal invocation. It will default to a yet unspecified value otherwise.
    * For bonus, we can have some fun with a heartbeat-based ADS discovery.

## Class structure in C++
All classes representing objects for use over the network should inherit from a superobject named `RemoteObject`. This doesn't maintain anything except mandate a virtual method.

```c++
virtual JSON executeRPC(JSON arguments) = 0;
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

## Work
* Message class
* Dispatcher class
    * RemoteObject class (thin abstract)
    * Example object
