-module(playerlang_app).

-behaviour(application).

-define(PLAYERLANG_LIB, playerlang_drv).
-define(CMD_PLAY, 2).

-export([start/2, stop/0, play/0]).

-ifdef(TEST).
-include_lib("eunit/include/eunit.hrl").
-endif.

start(_StartType, _StartArgs) ->
    case erl_ddll:load_driver("../ebin", ?PLAYERLANG_LIB) of
        ok ->
            Port = open_port({spawn, ?PLAYERLANG_LIB}, []),
            register(?PLAYERLANG_LIB, Port);
        {error, already_loaded} ->
            ok;
        {error, Message} -> exit(erl_ddll:format_error(Message))
    end,
    playerlang_sup:start_link().

stop() ->
    ok.

play() ->
    port_control(?PLAYERLANG_LIB, 2, ""),
    ok.

-ifdef(TEST).

play_test() ->
    application:start(playerlang),
    play().

-endif.
