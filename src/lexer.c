
#line 1 "lexer.rl"

#line 35 "lexer.rl"



#line 9 "lexer.c"
static const char _conf_lexer_actions[] = {
	0, 1, 1, 1, 3, 1, 4, 1, 
	5, 1, 14, 1, 15, 1, 16, 1, 
	17, 1, 19, 1, 20, 1, 21, 1, 
	22, 1, 23, 1, 24, 1, 25, 1, 
	26, 2, 0, 18, 2, 0, 19, 2, 
	2, 18, 2, 2, 19, 2, 5, 6, 
	2, 5, 7, 2, 5, 8, 2, 5, 
	9, 2, 5, 10, 2, 5, 11, 2, 
	5, 12, 2, 5, 13
};

static const short _conf_lexer_key_offsets[] = {
	0, 0, 3, 3, 5, 8, 8, 33, 
	34, 42, 53, 61, 71, 72, 83, 86, 
	94, 105, 117, 127, 137, 147, 157, 167, 
	177, 187, 197, 207, 217, 227, 239, 249, 
	259, 269, 279, 289, 299, 309, 319, 329
};

static const char _conf_lexer_trans_keys[] = {
	34, 92, 96, 10, 13, 39, 92, 96, 
	9, 10, 13, 32, 34, 35, 39, 58, 
	59, 76, 80, 83, 108, 112, 115, 123, 
	125, 33, 43, 45, 48, 49, 57, 63, 
	126, 10, 124, 126, 33, 43, 45, 58, 
	63, 122, 34, 92, 96, 124, 126, 33, 
	43, 45, 58, 63, 122, 124, 126, 33, 
	43, 45, 58, 63, 122, 10, 13, 124, 
	126, 33, 43, 45, 58, 63, 122, 10, 
	39, 92, 96, 124, 126, 33, 43, 45, 
	58, 63, 122, 39, 92, 96, 124, 126, 
	33, 43, 45, 58, 63, 122, 58, 124, 
	126, 33, 43, 45, 47, 48, 57, 63, 
	122, 73, 79, 105, 111, 124, 126, 33, 
	43, 45, 58, 63, 122, 83, 115, 124, 
	126, 33, 43, 45, 58, 63, 122, 84, 
	116, 124, 126, 33, 43, 45, 58, 63, 
	122, 69, 101, 124, 126, 33, 43, 45, 
	58, 63, 122, 78, 110, 124, 126, 33, 
	43, 45, 58, 63, 122, 71, 103, 124, 
	126, 33, 43, 45, 58, 63, 122, 82, 
	114, 124, 126, 33, 43, 45, 58, 63, 
	122, 73, 105, 124, 126, 33, 43, 45, 
	58, 63, 122, 77, 109, 124, 126, 33, 
	43, 45, 58, 63, 122, 65, 97, 124, 
	126, 33, 43, 45, 58, 63, 122, 82, 
	114, 124, 126, 33, 43, 45, 58, 63, 
	122, 89, 121, 124, 126, 33, 43, 45, 
	58, 63, 122, 69, 84, 101, 116, 124, 
	126, 33, 43, 45, 58, 63, 122, 67, 
	99, 124, 126, 33, 43, 45, 58, 63, 
	122, 79, 111, 124, 126, 33, 43, 45, 
	58, 63, 122, 78, 110, 124, 126, 33, 
	43, 45, 58, 63, 122, 68, 100, 124, 
	126, 33, 43, 45, 58, 63, 122, 65, 
	97, 124, 126, 33, 43, 45, 58, 63, 
	122, 82, 114, 124, 126, 33, 43, 45, 
	58, 63, 122, 89, 121, 124, 126, 33, 
	43, 45, 58, 63, 122, 79, 111, 124, 
	126, 33, 43, 45, 58, 63, 122, 82, 
	114, 124, 126, 33, 43, 45, 58, 63, 
	122, 69, 101, 124, 126, 33, 43, 45, 
	58, 63, 122, 0
};

static const char _conf_lexer_single_lengths[] = {
	0, 3, 0, 2, 3, 0, 17, 1, 
	2, 5, 2, 4, 1, 5, 3, 2, 
	3, 6, 4, 4, 4, 4, 4, 4, 
	4, 4, 4, 4, 4, 6, 4, 4, 
	4, 4, 4, 4, 4, 4, 4, 4
};

static const char _conf_lexer_range_lengths[] = {
	0, 0, 0, 0, 0, 0, 4, 0, 
	3, 3, 3, 3, 0, 3, 0, 3, 
	4, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3
};

static const short _conf_lexer_index_offsets[] = {
	0, 0, 4, 5, 8, 12, 13, 35, 
	37, 43, 52, 58, 66, 68, 77, 81, 
	87, 95, 105, 113, 121, 129, 137, 145, 
	153, 161, 169, 177, 185, 193, 203, 211, 
	219, 227, 235, 243, 251, 259, 267, 275
};

static const char _conf_lexer_indicies[] = {
	2, 3, 0, 1, 1, 5, 6, 4, 
	9, 10, 7, 8, 8, 11, 13, 14, 
	11, 16, 17, 18, 15, 20, 21, 22, 
	23, 21, 22, 23, 24, 25, 15, 15, 
	19, 15, 12, 27, 26, 15, 15, 15, 
	15, 15, 7, 29, 30, 15, 16, 16, 
	16, 16, 16, 1, 16, 16, 16, 16, 
	16, 1, 5, 6, 17, 17, 17, 17, 
	17, 4, 32, 31, 33, 34, 15, 18, 
	18, 18, 18, 18, 8, 9, 10, 35, 
	8, 18, 18, 18, 18, 18, 8, 15, 
	15, 15, 15, 15, 19, 15, 36, 37, 
	38, 37, 38, 15, 15, 15, 15, 15, 
	28, 39, 39, 15, 15, 15, 15, 15, 
	28, 40, 40, 15, 15, 15, 15, 15, 
	28, 41, 41, 15, 15, 15, 15, 15, 
	28, 42, 42, 15, 15, 15, 15, 15, 
	28, 43, 43, 15, 15, 15, 15, 15, 
	28, 44, 44, 15, 15, 15, 15, 15, 
	28, 45, 45, 15, 15, 15, 15, 15, 
	28, 46, 46, 15, 15, 15, 15, 15, 
	28, 47, 47, 15, 15, 15, 15, 15, 
	28, 48, 48, 15, 15, 15, 15, 15, 
	28, 49, 49, 15, 15, 15, 15, 15, 
	28, 50, 51, 50, 51, 15, 15, 15, 
	15, 15, 28, 52, 52, 15, 15, 15, 
	15, 15, 28, 53, 53, 15, 15, 15, 
	15, 15, 28, 54, 54, 15, 15, 15, 
	15, 15, 28, 55, 55, 15, 15, 15, 
	15, 15, 28, 56, 56, 15, 15, 15, 
	15, 15, 28, 57, 57, 15, 15, 15, 
	15, 15, 28, 58, 58, 15, 15, 15, 
	15, 15, 28, 59, 59, 15, 15, 15, 
	15, 15, 28, 60, 60, 15, 15, 15, 
	15, 15, 28, 61, 61, 15, 15, 15, 
	15, 15, 28, 0
};

static const char _conf_lexer_trans_targs[] = {
	6, 1, 6, 2, 3, 6, 12, 6, 
	4, 14, 5, 6, 0, 6, 7, 8, 
	9, 11, 13, 16, 6, 17, 23, 29, 
	6, 6, 6, 6, 6, 8, 10, 6, 
	6, 13, 15, 6, 6, 18, 22, 19, 
	20, 21, 8, 8, 24, 25, 26, 27, 
	28, 8, 30, 37, 31, 32, 33, 34, 
	35, 36, 8, 38, 39, 8
};

static const char _conf_lexer_trans_actions[] = {
	29, 0, 15, 0, 0, 33, 1, 31, 
	0, 63, 0, 17, 0, 36, 1, 66, 
	7, 7, 66, 0, 13, 0, 0, 0, 
	9, 11, 27, 42, 23, 60, 7, 25, 
	39, 63, 66, 21, 19, 0, 0, 0, 
	0, 0, 45, 51, 0, 0, 0, 0, 
	0, 54, 0, 0, 0, 0, 0, 0, 
	0, 0, 57, 0, 0, 48
};

static const char _conf_lexer_to_state_actions[] = {
	0, 0, 0, 0, 0, 0, 3, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0
};

static const char _conf_lexer_from_state_actions[] = {
	0, 0, 0, 0, 0, 0, 5, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0
};

static const short _conf_lexer_eof_trans[] = {
	0, 1, 1, 1, 8, 8, 0, 27, 
	8, 29, 29, 29, 32, 8, 36, 29, 
	37, 29, 29, 29, 29, 29, 29, 29, 
	29, 29, 29, 29, 29, 29, 29, 29, 
	29, 29, 29, 29, 29, 29, 29, 29
};

static const int conf_lexer_start = 6;
static const int conf_lexer_first_final = 6;
static const int conf_lexer_error = 0;

static const int conf_lexer_en_main = 6;


#line 38 "lexer.rl"

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

  
#line 213 "lexer.c"
	{
	cs = conf_lexer_start;
	ts = 0;
	te = 0;
	act = 0;
	}

#line 58 "lexer.rl"
  
#line 223 "lexer.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_acts = _conf_lexer_actions + _conf_lexer_from_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 4:
#line 1 "NONE"
	{ts = p;}
	break;
#line 244 "lexer.c"
		}
	}

	_keys = _conf_lexer_trans_keys + _conf_lexer_key_offsets[cs];
	_trans = _conf_lexer_index_offsets[cs];

	_klen = _conf_lexer_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _conf_lexer_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _conf_lexer_indicies[_trans];
_eof_trans:
	cs = _conf_lexer_trans_targs[_trans];

	if ( _conf_lexer_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _conf_lexer_actions + _conf_lexer_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 3 "lexer.rl"
	{ ctx->line++; }
	break;
	case 1:
#line 4 "lexer.rl"
	{ ctx->line++; }
	break;
	case 2:
#line 5 "lexer.rl"
	{ ctx->line--; }
	break;
	case 5:
#line 1 "NONE"
	{te = p+1;}
	break;
	case 6:
#line 17 "lexer.rl"
	{act = 2;}
	break;
	case 7:
#line 18 "lexer.rl"
	{act = 3;}
	break;
	case 8:
#line 19 "lexer.rl"
	{act = 4;}
	break;
	case 9:
#line 20 "lexer.rl"
	{act = 5;}
	break;
	case 10:
#line 21 "lexer.rl"
	{act = 6;}
	break;
	case 11:
#line 27 "lexer.rl"
	{act = 10;}
	break;
	case 12:
#line 28 "lexer.rl"
	{act = 11;}
	break;
	case 13:
#line 30 "lexer.rl"
	{act = 12;}
	break;
	case 14:
#line 23 "lexer.rl"
	{te = p+1;{ SYMBOL(TK_LP) }}
	break;
	case 15:
#line 24 "lexer.rl"
	{te = p+1;{ SYMBOL(TK_RP) }}
	break;
	case 16:
#line 25 "lexer.rl"
	{te = p+1;{ SYMBOL(TK_SEMICOLON) }}
	break;
	case 17:
#line 27 "lexer.rl"
	{te = p+1;{ ID() }}
	break;
	case 18:
#line 32 "lexer.rl"
	{te = p+1;}
	break;
	case 19:
#line 33 "lexer.rl"
	{te = p+1;}
	break;
	case 20:
#line 15 "lexer.rl"
	{te = p;p--;{ ID() }}
	break;
	case 21:
#line 28 "lexer.rl"
	{te = p;p--;{ ID() }}
	break;
	case 22:
#line 30 "lexer.rl"
	{te = p;p--;{ ID() }}
	break;
	case 23:
#line 32 "lexer.rl"
	{te = p;p--;}
	break;
	case 24:
#line 33 "lexer.rl"
	{te = p;p--;}
	break;
	case 25:
#line 30 "lexer.rl"
	{{p = ((te))-1;}{ ID() }}
	break;
	case 26:
#line 1 "NONE"
	{	switch( act ) {
	case 2:
	{{p = ((te))-1;} KEYWORD(TK_LISTEN) }
	break;
	case 3:
	{{p = ((te))-1;} KEYWORD(TK_STORE) }
	break;
	case 4:
	{{p = ((te))-1;} KEYWORD(TK_LOG) }
	break;
	case 5:
	{{p = ((te))-1;} KEYWORD(TK_BUFFER_TYPE) }
	break;
	case 6:
	{{p = ((te))-1;} KEYWORD(TK_BUFFER_TYPE) }
	break;
	case 10:
	{{p = ((te))-1;} ID() }
	break;
	case 11:
	{{p = ((te))-1;} ID() }
	break;
	case 12:
	{{p = ((te))-1;} ID() }
	break;
	}
	}
	break;
#line 436 "lexer.c"
		}
	}

_again:
	_acts = _conf_lexer_actions + _conf_lexer_to_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 3:
#line 1 "NONE"
	{ts = 0;}
	break;
#line 449 "lexer.c"
		}
	}

	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	if ( _conf_lexer_eof_trans[cs] > 0 ) {
		_trans = _conf_lexer_eof_trans[cs] - 1;
		goto _eof_trans;
	}
	}

	_out: {}
	}

#line 59 "lexer.rl"

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

