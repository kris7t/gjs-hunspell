
#include <string.h>
#include <iconv.h>

#include <gjs/gjs-module.h>

#include <hunspell/hunspell.hxx>

#include <iostream>

struct GjsHunspellSpell {
    Hunspell *hunhandle;
};

static inline GjsHunspellSpell *priv_from_js(JSContext *context,
					     JSObject *object,
					     jsval *argv);

static const char utf16_encoding[] = "UTF-16";

static JSString *
string_to_js_convert(JSContext *context,
		     const char *from_encoding,
		     char *str)
{
    char buf[500];

    char *inbuf = str;
    size_t inbytesleft = strlen(str);
    char *outbuf = &buf[0];
    size_t outbytesleft = sizeof(buf);
    
    iconv_t cd = iconv_open(utf16_encoding, from_encoding);
    iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
    iconv_close(cd);
    
    int outlen = (sizeof(buf) - outbytesleft) / 2;
    JSString *js = JS_NewUCStringCopyN(context,
				       reinterpret_cast<unsigned short *>(buf),
				       outlen);

    return js;
}
			   
static char *
js_to_string_convert(JSContext *context,
		     const char *to_encoding,
		     JSString *js)
{
    char buf[500];

    size_t length;
    const jschar *jsbuf = JS_GetStringCharsZAndLength(context, js, &length);
    
    char *inbuf = reinterpret_cast<char *>(const_cast<jschar *>(jsbuf));
    size_t inbytesleft = 2 * length;
    char *outbuf = &buf[0];
    size_t outbytesleft = sizeof(buf);

    iconv_t cd = iconv_open(to_encoding, utf16_encoding);
    iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
    iconv_close(cd);

    int outlen = sizeof(buf) - outbytesleft;
    char *result = new char[outlen + 1];
    memcpy(result, buf, outlen);
    result[outlen] = '\0';

    return result;
}

static JSObject *
string_vector_to_js(JSContext *context,
		    const char *from_encoding,
		    char **vector,
		    int n)
{
    jsval *jsVector = new jsval[n];

    for (int i = 0; i < n; ++i)
    {
	JSString *jsString = string_to_js_convert(context,
						  from_encoding,
						  vector[i]);
	jsVector[i] = STRING_TO_JSVAL(jsString);
    }


    JSObject *jsArray = JS_NewArrayObject(context, n, jsVector);

    return jsArray;
}

static void
delete_string_vector(char **vector,
		     int n)
{
    if (!vector)
	return;

    for (int i = 0; i < n; ++i)
	if (vector[i])
	    delete[] vector[i];

    delete[] vector;
}

static char **
js_to_string_vector(JSContext *context,
		    const char *from_encoding,
		    JSObject *js,
		    int *n)
{
    char **vector;

    if (!JS_IsArrayObject(context, js))
	goto fail;

    jsuint length;
    if (!JS_GetArrayLength(context, js, &length))
	goto fail;

    vector = new char *[length];
    memset(vector, 0, sizeof(vector));

    for (jsuint i = 0; i < length; ++i)
    {
	jsval jsValue;
	if (!JS_GetElement(context, js, i, &jsValue))
	    goto fail_with_vector;
	JSString *jsString = JS_ValueToString(context, jsValue);
	vector[i] = js_to_string_convert(context, from_encoding, jsString);
    }

    *n = length;
    return vector;

fail_with_vector:
    delete_string_vector(vector, length);

fail:
    *n = 0;
    return NULL;
}

#define GJS_HUNSPELL_METHOD_VARIABLES					\
    jsval *argv = JS_ARGV(context, vp);					\
    JSObject *object = JS_THIS_OBJECT(context, vp);			\
    GjsHunspellSpell *priv = priv_from_js(context, object, argv);

#define GJS_HUNSPELL_SUGGEST_PRELUDE					\
    GJS_HUNSPELL_METHOD_VARIABLES					\
									\
    if (priv == NULL)							\
	return JS_FALSE;						\
									\
    JSString *jsWord;							\
    if (!JS_ConvertArguments(context, argc, argv, "S", &jsWord)) {	\
	JS_ReportError(context,						\
		       "Expected String argument");			\
	return JS_FALSE;						\
    }									\
									\
    const char *dic_encoding = priv->hunhandle->get_dic_encoding();	\
									\
    char *word = js_to_string_convert(context, dic_encoding, jsWord);	\
									\
    char **slst;

#define GJS_HUNSPELL_SUGGEST_FINISH					\
    delete[] word;							\
    									\
    JSObject *jsArray = string_vector_to_js(context, dic_encoding,	\
					    slst, n);			\
    									\
    priv->hunhandle->free_list(&slst, n);				\
    									\
    JS_SET_RVAL(context, vp, OBJECT_TO_JSVAL(jsArray));

static const int gjs_hunspell_spell_tinyid_version = 1;

static const int gjs_hunspell_spell_tinyid_dic_encoding = 2;

static const int gjs_hunspell_spell_tinyid_wordchars = 3;

static JSBool
gjs_hunspell_spell_get_string_prop(JSContext *context,
				   JSObject *object,
				   jsid id,
				   jsval *vp)
{
    if (JSVAL_IS_STRING(*vp))
	return JS_TRUE;

    GjsHunspellSpell *priv = priv_from_js(context, object, NULL);

    jsval idval;
    if (!JS_IdToValue(context, id, &idval))
	return false;
    if (!JSVAL_IS_INT(idval))
	return false;
    int tinyid = JSVAL_TO_INT(idval);

    JSString *jsString = NULL;

    switch (tinyid) {
    case gjs_hunspell_spell_tinyid_version: {
	const char *version = priv->hunhandle->get_version();
	jsString = JS_NewStringCopyZ(context, version);
	break;
    }
    case gjs_hunspell_spell_tinyid_dic_encoding: {
	const char *dic_encoding = priv->hunhandle->get_dic_encoding();
	jsString = JS_NewStringCopyZ(context, dic_encoding);
	break;
    }
    case gjs_hunspell_spell_tinyid_wordchars: {
	int len;
	unsigned short *wordchars = priv->hunhandle->get_wordchars_utf16(&len);
	jsString = JS_NewUCStringCopyN(context, wordchars, len);
	break;
    }
    }

    if (jsString)
	JS_SET_RVAL(context, vp, STRING_TO_JSVAL(jsString));
    else
	JS_SET_RVAL(context, vp, JSVAL_VOID);
    
    return JS_TRUE;
}

static JSBool
gjs_hunspell_spell_spell_impl(JSContext *context,
			      JSObject *object,
			      uintN argc,
			      jsval *vp)
{
    jsval *argv = JS_ARGV(context, vp);

    GjsHunspellSpell *priv = priv_from_js(context, object, NULL);

    if (priv == NULL)
	return JS_FALSE;
    
    JSString *jsWord;
    if (!JS_ConvertArguments(context, argc, argv, "S", &jsWord)) {
	JS_ReportError(context,
		       "Expected String argument");
	return JS_FALSE;
    }

    const char *dic_encoding = priv->hunhandle->get_dic_encoding();

    char *word = js_to_string_convert(context, dic_encoding, jsWord);

    char *root;
    int info;
    int correct = priv->hunhandle->spell(word, &info, &root);

    delete[] word;

    if (correct) {
	JSObject *hash = JS_NewObject(context, NULL, NULL, NULL);

	JS_DefineProperty(context, hash, "compound",
			  (info & SPELL_COMPOUND) ? JSVAL_TRUE : JSVAL_FALSE,
			  NULL, NULL, JSPROP_ENUMERATE);

	JS_DefineProperty(context, hash, "forbidden",
			  (info & SPELL_FORBIDDEN) ? JSVAL_TRUE : JSVAL_FALSE,
			  NULL, NULL, JSPROP_ENUMERATE);

	JS_DefineProperty(context, hash, "warn",
			  (info & SPELL_WARN) ? JSVAL_TRUE : JSVAL_FALSE,
			  NULL, NULL, JSPROP_ENUMERATE);

	if (root != NULL) {
	    JSString *jsRoot = string_to_js_convert(context, dic_encoding, root);
	    JS_DefineProperty(context, hash, "root", STRING_TO_JSVAL(jsRoot),
			      NULL, NULL, JSPROP_ENUMERATE);
	}

	JS_SET_RVAL(context, vp, OBJECT_TO_JSVAL(hash));
    } else
	JS_SET_RVAL(context, vp, JSVAL_FALSE);

    return JS_TRUE;
}

static JSBool
gjs_hunspell_spell_spell(JSContext *context,
			 uintN argc,
			 jsval *vp)
{
    JSObject *object = JS_THIS_OBJECT(context, vp);

    return gjs_hunspell_spell_spell_impl(context, object, argc, vp);
}

static JSBool
gjs_hunspell_spell_call(JSContext *context,
			 uintN argc,
			 jsval *vp)
{
    JSObject *object = JSVAL_TO_OBJECT(JS_CALLEE(context, vp));

    return gjs_hunspell_spell_spell_impl(context, object, argc, vp);
}

static JSBool
gjs_hunspell_spell_suggest(JSContext *context,
			   uintN argc,
			   jsval *vp)
{
    GJS_HUNSPELL_SUGGEST_PRELUDE;

    int n = priv->hunhandle->suggest(&slst, word);

    GJS_HUNSPELL_SUGGEST_FINISH;

    return JS_TRUE;
}

static JSBool
gjs_hunspell_spell_analyze(JSContext *context,
			   uintN argc,
			   jsval *vp)
{
    GJS_HUNSPELL_SUGGEST_PRELUDE;

    int n = priv->hunhandle->analyze(&slst, word);

    GJS_HUNSPELL_SUGGEST_FINISH;

    return JS_TRUE;
}

static JSBool
gjs_hunspell_spell_unimplemented(JSContext *context,
				 uintN argc,
				 jsval *vp)
{
    JS_ReportError(context, "this method is unimplemented");

    return JS_FALSE;
}

GJS_NATIVE_CONSTRUCTOR_DECLARE(hunspell_spell)
{
    GJS_NATIVE_CONSTRUCTOR_VARIABLES(hunspell_spell)

    GJS_NATIVE_CONSTRUCTOR_PRELUDE(hunspell_spell);

    GjsHunspellSpell *priv = g_slice_new0(GjsHunspellSpell);
    g_assert(priv_from_js(context, object, NULL) == NULL);
    JS_SetPrivate(context, object, priv);

    JSString *jsAffpath, *jsDpath, *jsKey;
    char *affpath = NULL, *dpath = NULL, *key = NULL;
    Hunspell *hunhandle;
    if (argc < 2)
    {
	JS_ReportError(context,
		       "Expected at least 2 arguments instead got %d arguments",
		       argc);
	goto fail;
    } else if (argc == 2) {
	if (!JS_ConvertArguments(context, argc, argv, "SS",
				 &jsAffpath, &jsDpath))
	    goto fail;
	affpath = JS_EncodeString(context, jsAffpath);
	dpath = JS_EncodeString(context, jsDpath);
	hunhandle = new Hunspell(affpath, dpath);
    } else { /* argc >= 3 */
	if (!JS_ConvertArguments(context, argc, argv, "SSS",
				 &jsAffpath, &jsDpath, &jsKey))
	    goto fail;
	affpath = JS_EncodeString(context, jsAffpath);
	dpath = JS_EncodeString(context, jsDpath);
	key = JS_EncodeString(context, jsKey);
	hunhandle = new Hunspell(affpath, dpath, key);
    }

    if (affpath)
	JS_free(context, affpath);
    if (dpath)
	JS_free(context, dpath);
    if (key)
	JS_free(context, key);

    priv->hunhandle = hunhandle;

    GJS_NATIVE_CONSTRUCTOR_FINISH(hunspell_spell);

    return JS_TRUE;

fail:
    g_slice_free(GjsHunspellSpell, priv);

    return JS_FALSE;
}

static void
gjs_hunspell_spell_finalize(JSContext *context,
			    JSObject *object)
{
    GjsHunspellSpell *priv =
      static_cast<GjsHunspellSpell *>(JS_GetPrivate(context, object));

    if (priv) {
	JS_SetPrivate(context, object, NULL);
	if (priv->hunhandle)
	    delete priv->hunhandle;
	g_slice_free(GjsHunspellSpell, priv);
    }
}

static JSPropertySpec gjs_hunspell_spell_proto_props[] = {
    {
	"version",
	gjs_hunspell_spell_tinyid_version,
	JSPROP_ENUMERATE |
	JSPROP_READONLY,
	gjs_hunspell_spell_get_string_prop,
	NULL
    },
    {
	"dic_encoding",
	gjs_hunspell_spell_tinyid_dic_encoding,
	JSPROP_ENUMERATE |
	JSPROP_READONLY,
	gjs_hunspell_spell_get_string_prop,
	NULL
    },
    {
	"wordchars",
	gjs_hunspell_spell_tinyid_wordchars,
	JSPROP_ENUMERATE |
	JSPROP_READONLY,
	gjs_hunspell_spell_get_string_prop,
	NULL
    },
    { NULL, 0, 0, NULL, NULL }
};

static JSFunctionSpec gjs_hunspell_spell_proto_funcs[] = {
    {
	"add_dic",
	gjs_hunspell_spell_unimplemented, // TODO
	1,
	JSPROP_ENUMERATE
	
    },
    {
	"spell",
	gjs_hunspell_spell_spell,
	1,
	JSPROP_ENUMERATE
	
    },
    {
	"suggest",
	gjs_hunspell_spell_suggest,
	1,
	JSPROP_ENUMERATE
    },
    {
	"analyze",
	gjs_hunspell_spell_analyze,
	1,
	JSPROP_ENUMERATE
	
    },
    {
	"stem",
	gjs_hunspell_spell_unimplemented, // TODO
	1,
	JSPROP_ENUMERATE
	
    },
    {
	"generate",
	gjs_hunspell_spell_unimplemented, // TODO
	1,
	JSPROP_ENUMERATE
	
    },
    {
	"add",
	gjs_hunspell_spell_unimplemented, // TODO
	1,
	JSPROP_ENUMERATE
	
    },
    {
	"add_with_affix",
	gjs_hunspell_spell_unimplemented, // TODO
	1,
	JSPROP_ENUMERATE
	
    },
    {
	"remove",
	gjs_hunspell_spell_unimplemented, // TODO
	1,
	JSPROP_ENUMERATE
	
    },
    { NULL, NULL, 0, 0 }
};

static JSClass gjs_hunspell_spell_class = {
    "Spell",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_StrictPropertyStub,
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    gjs_hunspell_spell_finalize,
    NULL,
    NULL,
    gjs_hunspell_spell_call,
    NULL,
    NULL,
    NULL,
    NULL
};

static inline GjsHunspellSpell *
priv_from_js(JSContext *context,
	     JSObject *object,
	     jsval *argv)
{
    void *priv = JS_GetInstancePrivate(context, object,
				       &gjs_hunspell_spell_class,
				       argv);
    return static_cast<GjsHunspellSpell *>(priv);
}


static JSBool
gjs_define_hunspell_stuff(JSContext *context,
			  JSObject *module)
{
    JSObject *global = gjs_get_import_global(context);

    jsval val;
    if (!gjs_object_get_property(context, global,
				 gjs_hunspell_spell_class.name, &val)) {
	JSObject *prototype = JS_InitClass(context, global, NULL,
					   &gjs_hunspell_spell_class,
					   gjs_hunspell_spell_constructor,
					   2,
					   &gjs_hunspell_spell_proto_props[0],
					   &gjs_hunspell_spell_proto_funcs[0],
					   NULL, NULL);

	if (prototype == NULL)
	    return JS_FALSE;
	
	if (!gjs_object_require_property(context, global, NULL,
					 gjs_hunspell_spell_class.name,
					 &val))
	    return JS_FALSE;

	if (!JS_DefineProperty(context, module,
			       gjs_hunspell_spell_class.name,
			       val, NULL, NULL,
			       GJS_MODULE_PROP_FLAGS))
	    return false;
    }

    return JS_TRUE;
}

__attribute__((constructor)) static void
register_native_module()
{
    gjs_register_native_module("hunspell",
			       gjs_define_hunspell_stuff,
			       static_cast<GjsNativeFlags>(0));
}
