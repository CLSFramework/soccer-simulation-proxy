// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: service.proto

#include "service.pb.h"
#include "service.grpc.pb.h"

#include <functional>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/impl/channel_interface.h>
#include <grpcpp/impl/client_unary_call.h>
#include <grpcpp/support/client_callback.h>
#include <grpcpp/support/message_allocator.h>
#include <grpcpp/support/method_handler.h>
#include <grpcpp/impl/rpc_service_method.h>
#include <grpcpp/support/server_callback.h>
#include <grpcpp/impl/server_callback_handlers.h>
#include <grpcpp/server_context.h>
#include <grpcpp/impl/service_type.h>
#include <grpcpp/support/sync_stream.h>
namespace protos {

static const char* Game_method_names[] = {
  "/protos.Game/GetPlayerActions",
  "/protos.Game/GetCoachActions",
  "/protos.Game/GetTrainerActions",
  "/protos.Game/SendInitMessage",
  "/protos.Game/SendServerParams",
  "/protos.Game/SendPlayerParams",
  "/protos.Game/SendPlayerType",
  "/protos.Game/GetInitMessage",
  "/protos.Game/SendByeCommand",
};

std::unique_ptr< Game::Stub> Game::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< Game::Stub> stub(new Game::Stub(channel, options));
  return stub;
}

Game::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_GetPlayerActions_(Game_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_GetCoachActions_(Game_method_names[1], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_GetTrainerActions_(Game_method_names[2], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_SendInitMessage_(Game_method_names[3], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_SendServerParams_(Game_method_names[4], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_SendPlayerParams_(Game_method_names[5], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_SendPlayerType_(Game_method_names[6], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_GetInitMessage_(Game_method_names[7], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_SendByeCommand_(Game_method_names[8], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status Game::Stub::GetPlayerActions(::grpc::ClientContext* context, const ::protos::State& request, ::protos::PlayerActions* response) {
  return ::grpc::internal::BlockingUnaryCall< ::protos::State, ::protos::PlayerActions, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetPlayerActions_, context, request, response);
}

void Game::Stub::async::GetPlayerActions(::grpc::ClientContext* context, const ::protos::State* request, ::protos::PlayerActions* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::protos::State, ::protos::PlayerActions, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetPlayerActions_, context, request, response, std::move(f));
}

void Game::Stub::async::GetPlayerActions(::grpc::ClientContext* context, const ::protos::State* request, ::protos::PlayerActions* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetPlayerActions_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::protos::PlayerActions>* Game::Stub::PrepareAsyncGetPlayerActionsRaw(::grpc::ClientContext* context, const ::protos::State& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::protos::PlayerActions, ::protos::State, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetPlayerActions_, context, request);
}

::grpc::ClientAsyncResponseReader< ::protos::PlayerActions>* Game::Stub::AsyncGetPlayerActionsRaw(::grpc::ClientContext* context, const ::protos::State& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetPlayerActionsRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Game::Stub::GetCoachActions(::grpc::ClientContext* context, const ::protos::State& request, ::protos::CoachActions* response) {
  return ::grpc::internal::BlockingUnaryCall< ::protos::State, ::protos::CoachActions, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetCoachActions_, context, request, response);
}

void Game::Stub::async::GetCoachActions(::grpc::ClientContext* context, const ::protos::State* request, ::protos::CoachActions* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::protos::State, ::protos::CoachActions, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetCoachActions_, context, request, response, std::move(f));
}

void Game::Stub::async::GetCoachActions(::grpc::ClientContext* context, const ::protos::State* request, ::protos::CoachActions* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetCoachActions_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::protos::CoachActions>* Game::Stub::PrepareAsyncGetCoachActionsRaw(::grpc::ClientContext* context, const ::protos::State& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::protos::CoachActions, ::protos::State, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetCoachActions_, context, request);
}

::grpc::ClientAsyncResponseReader< ::protos::CoachActions>* Game::Stub::AsyncGetCoachActionsRaw(::grpc::ClientContext* context, const ::protos::State& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetCoachActionsRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Game::Stub::GetTrainerActions(::grpc::ClientContext* context, const ::protos::State& request, ::protos::TrainerActions* response) {
  return ::grpc::internal::BlockingUnaryCall< ::protos::State, ::protos::TrainerActions, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetTrainerActions_, context, request, response);
}

void Game::Stub::async::GetTrainerActions(::grpc::ClientContext* context, const ::protos::State* request, ::protos::TrainerActions* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::protos::State, ::protos::TrainerActions, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetTrainerActions_, context, request, response, std::move(f));
}

void Game::Stub::async::GetTrainerActions(::grpc::ClientContext* context, const ::protos::State* request, ::protos::TrainerActions* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetTrainerActions_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::protos::TrainerActions>* Game::Stub::PrepareAsyncGetTrainerActionsRaw(::grpc::ClientContext* context, const ::protos::State& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::protos::TrainerActions, ::protos::State, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetTrainerActions_, context, request);
}

::grpc::ClientAsyncResponseReader< ::protos::TrainerActions>* Game::Stub::AsyncGetTrainerActionsRaw(::grpc::ClientContext* context, const ::protos::State& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetTrainerActionsRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Game::Stub::SendInitMessage(::grpc::ClientContext* context, const ::protos::InitMessage& request, ::protos::Empty* response) {
  return ::grpc::internal::BlockingUnaryCall< ::protos::InitMessage, ::protos::Empty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_SendInitMessage_, context, request, response);
}

void Game::Stub::async::SendInitMessage(::grpc::ClientContext* context, const ::protos::InitMessage* request, ::protos::Empty* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::protos::InitMessage, ::protos::Empty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_SendInitMessage_, context, request, response, std::move(f));
}

void Game::Stub::async::SendInitMessage(::grpc::ClientContext* context, const ::protos::InitMessage* request, ::protos::Empty* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_SendInitMessage_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::protos::Empty>* Game::Stub::PrepareAsyncSendInitMessageRaw(::grpc::ClientContext* context, const ::protos::InitMessage& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::protos::Empty, ::protos::InitMessage, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_SendInitMessage_, context, request);
}

::grpc::ClientAsyncResponseReader< ::protos::Empty>* Game::Stub::AsyncSendInitMessageRaw(::grpc::ClientContext* context, const ::protos::InitMessage& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncSendInitMessageRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Game::Stub::SendServerParams(::grpc::ClientContext* context, const ::protos::ServerParam& request, ::protos::Empty* response) {
  return ::grpc::internal::BlockingUnaryCall< ::protos::ServerParam, ::protos::Empty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_SendServerParams_, context, request, response);
}

void Game::Stub::async::SendServerParams(::grpc::ClientContext* context, const ::protos::ServerParam* request, ::protos::Empty* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::protos::ServerParam, ::protos::Empty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_SendServerParams_, context, request, response, std::move(f));
}

void Game::Stub::async::SendServerParams(::grpc::ClientContext* context, const ::protos::ServerParam* request, ::protos::Empty* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_SendServerParams_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::protos::Empty>* Game::Stub::PrepareAsyncSendServerParamsRaw(::grpc::ClientContext* context, const ::protos::ServerParam& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::protos::Empty, ::protos::ServerParam, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_SendServerParams_, context, request);
}

::grpc::ClientAsyncResponseReader< ::protos::Empty>* Game::Stub::AsyncSendServerParamsRaw(::grpc::ClientContext* context, const ::protos::ServerParam& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncSendServerParamsRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Game::Stub::SendPlayerParams(::grpc::ClientContext* context, const ::protos::PlayerParam& request, ::protos::Empty* response) {
  return ::grpc::internal::BlockingUnaryCall< ::protos::PlayerParam, ::protos::Empty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_SendPlayerParams_, context, request, response);
}

void Game::Stub::async::SendPlayerParams(::grpc::ClientContext* context, const ::protos::PlayerParam* request, ::protos::Empty* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::protos::PlayerParam, ::protos::Empty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_SendPlayerParams_, context, request, response, std::move(f));
}

void Game::Stub::async::SendPlayerParams(::grpc::ClientContext* context, const ::protos::PlayerParam* request, ::protos::Empty* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_SendPlayerParams_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::protos::Empty>* Game::Stub::PrepareAsyncSendPlayerParamsRaw(::grpc::ClientContext* context, const ::protos::PlayerParam& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::protos::Empty, ::protos::PlayerParam, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_SendPlayerParams_, context, request);
}

::grpc::ClientAsyncResponseReader< ::protos::Empty>* Game::Stub::AsyncSendPlayerParamsRaw(::grpc::ClientContext* context, const ::protos::PlayerParam& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncSendPlayerParamsRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Game::Stub::SendPlayerType(::grpc::ClientContext* context, const ::protos::PlayerType& request, ::protos::Empty* response) {
  return ::grpc::internal::BlockingUnaryCall< ::protos::PlayerType, ::protos::Empty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_SendPlayerType_, context, request, response);
}

void Game::Stub::async::SendPlayerType(::grpc::ClientContext* context, const ::protos::PlayerType* request, ::protos::Empty* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::protos::PlayerType, ::protos::Empty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_SendPlayerType_, context, request, response, std::move(f));
}

void Game::Stub::async::SendPlayerType(::grpc::ClientContext* context, const ::protos::PlayerType* request, ::protos::Empty* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_SendPlayerType_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::protos::Empty>* Game::Stub::PrepareAsyncSendPlayerTypeRaw(::grpc::ClientContext* context, const ::protos::PlayerType& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::protos::Empty, ::protos::PlayerType, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_SendPlayerType_, context, request);
}

::grpc::ClientAsyncResponseReader< ::protos::Empty>* Game::Stub::AsyncSendPlayerTypeRaw(::grpc::ClientContext* context, const ::protos::PlayerType& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncSendPlayerTypeRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Game::Stub::GetInitMessage(::grpc::ClientContext* context, const ::protos::Empty& request, ::protos::InitMessageFromServer* response) {
  return ::grpc::internal::BlockingUnaryCall< ::protos::Empty, ::protos::InitMessageFromServer, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetInitMessage_, context, request, response);
}

void Game::Stub::async::GetInitMessage(::grpc::ClientContext* context, const ::protos::Empty* request, ::protos::InitMessageFromServer* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::protos::Empty, ::protos::InitMessageFromServer, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetInitMessage_, context, request, response, std::move(f));
}

void Game::Stub::async::GetInitMessage(::grpc::ClientContext* context, const ::protos::Empty* request, ::protos::InitMessageFromServer* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetInitMessage_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::protos::InitMessageFromServer>* Game::Stub::PrepareAsyncGetInitMessageRaw(::grpc::ClientContext* context, const ::protos::Empty& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::protos::InitMessageFromServer, ::protos::Empty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetInitMessage_, context, request);
}

::grpc::ClientAsyncResponseReader< ::protos::InitMessageFromServer>* Game::Stub::AsyncGetInitMessageRaw(::grpc::ClientContext* context, const ::protos::Empty& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetInitMessageRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Game::Stub::SendByeCommand(::grpc::ClientContext* context, const ::protos::Empty& request, ::protos::Empty* response) {
  return ::grpc::internal::BlockingUnaryCall< ::protos::Empty, ::protos::Empty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_SendByeCommand_, context, request, response);
}

void Game::Stub::async::SendByeCommand(::grpc::ClientContext* context, const ::protos::Empty* request, ::protos::Empty* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::protos::Empty, ::protos::Empty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_SendByeCommand_, context, request, response, std::move(f));
}

void Game::Stub::async::SendByeCommand(::grpc::ClientContext* context, const ::protos::Empty* request, ::protos::Empty* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_SendByeCommand_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::protos::Empty>* Game::Stub::PrepareAsyncSendByeCommandRaw(::grpc::ClientContext* context, const ::protos::Empty& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::protos::Empty, ::protos::Empty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_SendByeCommand_, context, request);
}

::grpc::ClientAsyncResponseReader< ::protos::Empty>* Game::Stub::AsyncSendByeCommandRaw(::grpc::ClientContext* context, const ::protos::Empty& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncSendByeCommandRaw(context, request, cq);
  result->StartCall();
  return result;
}

Game::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Game_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Game::Service, ::protos::State, ::protos::PlayerActions, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Game::Service* service,
             ::grpc::ServerContext* ctx,
             const ::protos::State* req,
             ::protos::PlayerActions* resp) {
               return service->GetPlayerActions(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Game_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Game::Service, ::protos::State, ::protos::CoachActions, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Game::Service* service,
             ::grpc::ServerContext* ctx,
             const ::protos::State* req,
             ::protos::CoachActions* resp) {
               return service->GetCoachActions(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Game_method_names[2],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Game::Service, ::protos::State, ::protos::TrainerActions, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Game::Service* service,
             ::grpc::ServerContext* ctx,
             const ::protos::State* req,
             ::protos::TrainerActions* resp) {
               return service->GetTrainerActions(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Game_method_names[3],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Game::Service, ::protos::InitMessage, ::protos::Empty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Game::Service* service,
             ::grpc::ServerContext* ctx,
             const ::protos::InitMessage* req,
             ::protos::Empty* resp) {
               return service->SendInitMessage(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Game_method_names[4],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Game::Service, ::protos::ServerParam, ::protos::Empty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Game::Service* service,
             ::grpc::ServerContext* ctx,
             const ::protos::ServerParam* req,
             ::protos::Empty* resp) {
               return service->SendServerParams(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Game_method_names[5],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Game::Service, ::protos::PlayerParam, ::protos::Empty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Game::Service* service,
             ::grpc::ServerContext* ctx,
             const ::protos::PlayerParam* req,
             ::protos::Empty* resp) {
               return service->SendPlayerParams(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Game_method_names[6],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Game::Service, ::protos::PlayerType, ::protos::Empty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Game::Service* service,
             ::grpc::ServerContext* ctx,
             const ::protos::PlayerType* req,
             ::protos::Empty* resp) {
               return service->SendPlayerType(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Game_method_names[7],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Game::Service, ::protos::Empty, ::protos::InitMessageFromServer, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Game::Service* service,
             ::grpc::ServerContext* ctx,
             const ::protos::Empty* req,
             ::protos::InitMessageFromServer* resp) {
               return service->GetInitMessage(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Game_method_names[8],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Game::Service, ::protos::Empty, ::protos::Empty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Game::Service* service,
             ::grpc::ServerContext* ctx,
             const ::protos::Empty* req,
             ::protos::Empty* resp) {
               return service->SendByeCommand(ctx, req, resp);
             }, this)));
}

Game::Service::~Service() {
}

::grpc::Status Game::Service::GetPlayerActions(::grpc::ServerContext* context, const ::protos::State* request, ::protos::PlayerActions* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Game::Service::GetCoachActions(::grpc::ServerContext* context, const ::protos::State* request, ::protos::CoachActions* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Game::Service::GetTrainerActions(::grpc::ServerContext* context, const ::protos::State* request, ::protos::TrainerActions* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Game::Service::SendInitMessage(::grpc::ServerContext* context, const ::protos::InitMessage* request, ::protos::Empty* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Game::Service::SendServerParams(::grpc::ServerContext* context, const ::protos::ServerParam* request, ::protos::Empty* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Game::Service::SendPlayerParams(::grpc::ServerContext* context, const ::protos::PlayerParam* request, ::protos::Empty* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Game::Service::SendPlayerType(::grpc::ServerContext* context, const ::protos::PlayerType* request, ::protos::Empty* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Game::Service::GetInitMessage(::grpc::ServerContext* context, const ::protos::Empty* request, ::protos::InitMessageFromServer* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Game::Service::SendByeCommand(::grpc::ServerContext* context, const ::protos::Empty* request, ::protos::Empty* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace protos

