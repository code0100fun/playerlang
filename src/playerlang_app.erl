-module(playerlang_app).

-behaviour(application).

-define(PLAYERLANG_LIB, "playerlang").

%% Application callbacks
-export([start/2, stop/1]).

-ifdef(TEST).
-include_lib("eunit/include/eunit.hrl").
-endif.

%% ===================================================================
%% Application callbacks
%% ===================================================================

start(_StartType, _StartArgs) ->
    case erl_ddll:load_driver("../ebin", playerlang_drv) of
        ok -> ok;
        {error, already_loaded} -> ok;
        {error, Message} -> exit(erl_ddll:format_error(Message))
    end,
    % spawn(?MODULE, init, [?PLAYERLANG_LIB]),
    playerlang_sup:start_link().

% init() ->
%     % register(play, self()),
%     % Port = open_port({spawn, ?PLAYERLANG_LIB}, [])
%     ok.

stop(_State) ->
    ok.

-ifdef(TEST).

simple_test() ->
        ok = application:start(playerlang),
            ?assertNot(undefined == whereis(playerlang_sup)).

-endif.
