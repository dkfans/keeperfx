```puml
Plantuml-like
Client -> Server: PckA_Login + name + password
Server -> Client: PckA_Login + NetUserId "Your Id"
Server -> <All>: PckA_UserUpdate + NetUserId "His Id" + connection_status + name
...
Server -> <all>: PckA_LandviewFrameSrv + old shit
Client -> <all>: PckA_LandviewFrameSrv + old shit
```