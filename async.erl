-module(async).

-export([start/0]).
-export([create/1, stop/0, putdb/3, call_port/3, process/0]).

-define(PUT, $p).
-define(PROCESS, $r).

start() ->
	case erl_ddll:load_driver(".", "async_drv") of
		ok -> ok;
		{error, already_loaded} -> ok;
		{error, Message} -> exit(erl_ddll:format_error(Message))
	end,
	spawn(async, create, [self()]),
	
	receive 
		Port ->
			Port
	end.

create(Pid) ->
	register(async, self()),
	Port = open_port({spawn_driver, async_drv}, [binary]),
	
	% create an ETS table in order to store the PID
	% we used to return the PID and let the caller to everything
	% but this is cleaner
	ets:new(async_table, [named_table, protected, set, {keypos, 1}, {read_concurrency, true}]),
	ets:insert(async_table, {port, Port}),

	Pid ! Port,
	loop(Port).


call_port(Command, Key, Value) ->
	async ! {Command, self(), Key, Value},
	receive
		{ok, Result} ->
			{ok, Result};
		Error ->
			io:format("ERROR ~p ~n", [Error]),
			{ok, retry}
	end.

stop() ->
	async ! stop,
	unregister(async).

process() ->
	call_port(?PROCESS, '_', '_').

putdb(AppKey, Value, Port) ->	
%	[{_, Port}] = ets:lookup(async_table, port),
	port_command(Port, term_to_binary({?PUT, AppKey, Value})).


async_response(Caller) ->
	receive 
		Data ->
			Caller ! Data
	after
		5000 ->
		    % failed to get a response from lethe
			failed
	end.

	
 loop(Port) ->
 	receive
 		{Command, Caller, Key, Value} ->
 		%	[{_, Port}] = ets:lookup(async_table, port),
 			port_command(Port, term_to_binary({Command, Key, Value})),
 			async_response(Caller),
 			loop(Port);
 		stop ->
 		%	[{_, Port}] = ets:lookup(async_table, port),
 			Port ! {self(), close},
 			receive
 				{closed} ->
 					exit(normal)
 			end
 	end.


