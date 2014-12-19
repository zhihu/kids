%include {
  #include <assert.h>
}

%token_destructor { ((void)ctx); delete $$; }

%parse_failure {
  ctx->success = false;
  snprintf(ctx->error, 1024, "%s(line: %d)", ERR_SYNTAX, ctx->line);
}

%syntax_error {
  if (TOKEN) {
    snprintf(ctx->error, 1024, "%s(line: %d): %s", ERR_SYNTAX, ctx->line, TOKEN->value.c_str());
  } else {
    snprintf(ctx->error, 1024, "%s(line: %d)", ERR_SYNTAX, ctx->line);
  }
  ctx->success = false;
}

%stack_overflow {
  snprintf(ctx->error, 1024, "parse config file stack overflow(%d)", ctx->line);
  ctx->success = false;
}

%extra_argument {ParseContext *ctx}

%token_type {Token*}
%token_prefix TK_

%type conf {KidsConfig*}
%type log {KidsConfig*}
%type listen {KidsConfig*}
%type store {StoreConfig*}
%type keyvalue {KeyValue*}
%type value {std::vector<std::string>*}
%type log_conf {KidsConfig*}
%type listen_conf {KidsConfig*}
%type store_conf {StoreConfig*}
%type buffer_type {Token*}

main ::= conf(A). {
  ctx->conf = A;
}

value(A) ::= ID(V). {
  A = new std::vector<std::string>();
  A->push_back(V->value);
  delete V;
}
value(A) ::= value(B) ID(V). {
  A = B;
  B->push_back(V->value);
  delete V;
}

keyvalue(A) ::= ID(K) value(V) SEMICOLON. {
  A = new KeyValue(K->value, V);
  delete K;
  delete V;
}

conf(A) ::= keyvalue(B). {
  A = new KidsConfig();;
  SetKidsConfValue(A, B, ctx);
  delete B;
}
conf(A) ::= conf(B) keyvalue(C). {
  A = B;
  SetKidsConfValue(A, C, ctx);
  delete C;
}

conf(A) ::= listen(B). {
  A = B;
}
conf(A) ::= conf(B) listen(C). {
  A = B;
  if (!A->listen_socket.empty() || !A->listen_host.empty()) {
    snprintf(ctx->error, 1024, ERR_MULTI_LISTEN_HOST);
    ctx->success = false;
  } else {
    A->listen_socket = C->listen_socket;
    A->listen_host = C->listen_host;
    A->listen_port = C->listen_port;
  }
  delete C;
}

conf(A) ::= store(B). {
  A = new KidsConfig();
  A->store = B;
}
conf(A) ::= conf(B) store(C). {
  A = B;
  if (A->store != NULL) {
    snprintf(ctx->error, 1024, ERR_MULTI_STORE);
    ctx->success = false;
    delete C;
  } else {
    A->store = C;
  }
}

conf(A) ::= log(B). {
  A = B;
}
conf(A) ::= conf(B) log(C). {
  A = B;
  A->log_level = C->log_level;
  A->log_file = C->log_file;
  delete C;
}

log(A) ::= LOG LP log_conf(B) RP. {
  A = B;
}
log_conf(A) ::= keyvalue(B). {
  A = new KidsConfig();
  SetKidsConfValue(A, B, ctx);
  delete B;
}
log_conf(A) ::= log_conf(B) keyvalue(C). {
  A = B;
  SetKidsConfValue(A, C, ctx);
  delete C;
}

listen(A) ::= LISTEN LP listen_conf(B) RP. {
  A = B;
}
listen_conf(A) ::= keyvalue(B). {
  A = new KidsConfig();
  SetKidsConfValue(A, B, ctx);
  delete B;
}
listen_conf(A) ::= listen_conf(B) keyvalue(C). {
  A = B;
  SetKidsConfValue(A, C, ctx);
  delete C;
}

store(A) ::= STORE ID(B) buffer_type(C) LP store_conf(D) RP. {
  A = D;
  A->type = B->value;
  if (C != NULL) {
    A->buffer_type = C->value;
  } else {
    A->buffer_type.clear();
  }
  delete B;
  delete C;
}

store(A) ::= STORE ID(B) buffer_type(C) LP RP. {
  A = new StoreConfig();
  A->type = B->value;
  if (C != NULL) {
    A->buffer_type = C->value;
  } else {
    A->buffer_type.clear();
  }
  delete B;
  delete C;
}

buffer_type(A) ::= BUFFER_TYPE(B). {
  A = B;
}
buffer_type ::=.

store_conf(A) ::= keyvalue(B). {
  A = new StoreConfig();
  SetStoreConfValue(A, B, ctx);
  delete B;
}
store_conf(A) ::= store_conf(B) keyvalue(C). {
  A = B;
  SetStoreConfValue(A, C, ctx);
  delete C;
}
store_conf(A) ::= store(B). {
  A = new StoreConfig();
  A->stores.push_back(B);
}
store_conf(A) ::= store_conf(B) store(C). {
  A = B;
  A->stores.push_back(C);
}

