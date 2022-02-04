/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    client.cpp
 * @date    03.01.2013
 * @author  Peter Spiess-Knafl <dev@spiessknafl.at>
 * @license See attached LICENSE.txt
 ************************************************************************/

#include "client.h"
#include "rpcprotocolclient.h"

using namespace jsonrpc;

Client::Client(IClientConnector &connector, clientVersion_t version,
               bool omitEndingLineFeed)
    : connector(connector) {
  this->protocol = new RpcProtocolClient(version, omitEndingLineFeed);
}

Client::~Client() { delete this->protocol; }

void Client::CallMethod(const std::string &name, const Json::Value &parameter,
                        Json::Value &result, std::stringstream &oss) {
  std::string request, response;
  oss << "HI1 ";
  protocol->BuildRequest(name, parameter, request, false, oss);
  connector.SendRPCMessage(request, response, oss);
  protocol->HandleResponse(response, result, oss);
  oss << "HI2 ";
}

void Client::CallProcedures(const BatchCall &calls, BatchResponse &result) {
  std::string request, response;
  request = calls.toString();
  std::stringstream oss;
  connector.SendRPCMessage(request, response, oss);
  Json::Reader reader;
  Json::Value tmpresult;

  try {
    if (!reader.parse(response, tmpresult) || !tmpresult.isArray()) {
      throw JsonRpcException(Errors::ERROR_CLIENT_INVALID_RESPONSE,
                             "Array expected.");
    }
  } catch (const Json::Exception &e) {
    throw JsonRpcException(
        Errors::ERROR_RPC_JSON_PARSE_ERROR,
        Errors::GetErrorMessage(Errors::ERROR_RPC_JSON_PARSE_ERROR), response);
  }

  for (unsigned int i = 0; i < tmpresult.size(); i++) {
    if (tmpresult[i].isObject()) {
      Json::Value singleResult;
      try {
        Json::Value id =
            this->protocol->HandleResponse(tmpresult[i], singleResult, oss);
        result.addResponse(id, singleResult, false);
      } catch (JsonRpcException &ex) {
        Json::Value id = -1;
        if (tmpresult[i].isMember("id"))
          id = tmpresult[i]["id"];
        result.addResponse(id, tmpresult[i]["error"], true);
      }
    } else
      throw JsonRpcException(Errors::ERROR_CLIENT_INVALID_RESPONSE,
                             "Object in Array expected.");
  }
}

BatchResponse Client::CallProcedures(const BatchCall &calls) {
  BatchResponse result;
  this->CallProcedures(calls, result);
  return result;
}

Json::Value Client::CallMethod(const std::string &name,
                               const Json::Value &parameter,
                               std::stringstream &oss) {
  Json::Value result;
  oss << "HIC1 ";
  this->CallMethod(name, parameter, result, oss);
  oss << "HIC2 ";
  return result;
}

void Client::CallNotification(const std::string &name,
                              const Json::Value &parameter) {
  std::string request, response;
  std::stringstream oss;
  protocol->BuildRequest(name, parameter, request, true, oss);
  connector.SendRPCMessage(request, response, oss);
}
