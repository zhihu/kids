%%{
  machine conf_lexer;
  line_sane = ( '\n' ) >{ ctx->line++; };
  line_weird = ( '\r' ) >{ ctx->line++; };
  line_insane = ( '\r' ( '\n' >{ ctx->line--; } ) );
  line = ( line_sane | line_weird | line_insane );

  ws = ( '\t' | ' ' );
  comment = ( '#' (any - line)* line );
  noise = ( ws | line | comment );

  dliteralChar = [^`"\\] | ( '\\' any );

  main := |*
    [1-9][0-9]* { ID() };

    /listen/i { KEYWORD(TK_LISTEN) };
    /store/i { KEYWORD(TK_STORE) };
    /log/i { KEYWORD(TK_LOG) };
    /primary/i { KEYWORD(TK_BUFFER_TYPE) };
    /secondary/i { KEYWORD(TK_BUFFER_TYPE) };

    '{' { SYMBOL(TK_LP) };
    '}' { SYMBOL(TK_RP) };
    ';' { SYMBOL(TK_SEMICOLON) };

    '"' (dliteralChar | "'")* '"' { ID() };
    "'" (dliteralChar | '"')* "'" { ID() };

    (print - noise - [,<>=;{}])+ { ID() };

    comment;
    noise;
  *|;
}%%

%% write data;

ParseContext *ParseConfig(std::string str) {
  char *p = const_cast<char*>(str.c_str());
  char *pe = p + str.length();
  char *eof = pe;
  int cs, act;
  char *ts, *te;

  ParseContext *ctx = new ParseContext();
  if (str.empty()) {
    ctx->success = false;
    snprintf(ctx->error, 1024, ERR_BAD_CONF_FILE);
    return ctx;
  }

  Token *tk = NULL;

  void *parser = ParseAlloc(malloc);

  %% write init;
  %% write exec;

  if (cs < conf_lexer_first_final) {
    snprintf(ctx->error, 1024, "%s(line: %d)", ERR_SYNTAX, ctx->line);
    ctx->success = false;
  } else {
    Parse(parser, 0, 0, ctx);
  }

  ParseFree(parser, free);

  if (ctx->success) {
    CheckKidsConfig(ctx->conf, ctx);
  }
  return ctx;
}

