#include <stdio.h>

/* Specification. */
#include "output-json.h"

#include <assert.h> /* defines assert() */
#include <ctype.h>  /* declares isprint() */
#include <stdio.h>
#include "options.h"
#include "version.h"

OutputJSON::OutputJSON (KeywordExt_List *head,
                                    const char *struct_decl,
                                    unsigned int struct_decl_lineno,
                                    const char *return_type,
                                    const char *struct_tag,
                                    const char *verbatim_declarations,
                                    const char *verbatim_declarations_end,
                                    unsigned int verbatim_declarations_lineno,
                                    const char *verbatim_code,
                                    const char *verbatim_code_end,
                                    unsigned int verbatim_code_lineno,
                                    bool charset_dependent,
                                    int total_keys,
                                    int max_key_len, int min_key_len,
                                    bool hash_includes_len,
                                    const Positions& positions,
                                    const unsigned int *alpha_inc,
                                    int total_duplicates,
                                    unsigned int alpha_size,
                                    const int *asso_values)
  : Output(head, struct_decl, struct_decl_lineno, return_type, struct_tag,
      verbatim_declarations, verbatim_declarations_end,
      verbatim_declarations_lineno, verbatim_code, verbatim_code_end,
      verbatim_code_lineno, charset_dependent, total_keys, max_key_len,
      min_key_len, hash_includes_len, positions, alpha_inc, total_duplicates,
      alpha_size, asso_values)
{
}


/* Outputs a keyword, as a string: enclosed in double quotes, escaping
   backslashes, double quote and unprintable characters.  */

static void
output_string (const char *key, int len)
{
  putchar ('"');
  for (; len > 0; len--)
    {
      unsigned char c = static_cast<unsigned char>(*key++);
      if (isprint (c))
        {
          if (c == '"' || c == '\\')
            putchar ('\\');
          putchar (c);
        }
      else
        {
          printf("\\u%04x", c);
        }
    }
  putchar ('"');
}

static void
output_string (const char *key)
{
  putchar ('"');
  while(unsigned char c = static_cast<unsigned char>(*key++))
    {
      if (isprint (c))
        {
          if (c == '"' || c == '\\')
            putchar ('\\');
          putchar (c);
        }
      else
        {
          printf("\\u%04x", c);
        }
    }
  putchar ('"');
}

static void output_nulls(const char *indent, int count) {


  for (int i = 0; i < count; ++i) {
    if ((i % 10) == 0) {
      if (i) fputc('\n', stdout);
      fputs(indent, stdout);
    }
    fputs("null, ", stdout);
  }
  printf("\n");
}

static void
output_keyword_entry (KeywordExt *keyword)
{

  fputs("    [", stdout);
  output_string (keyword->_allchars, keyword->_allchars_length);

  printf(", %d", keyword->_hash_value);

  if (keyword->_rest && *keyword->_rest) {
    printf(", ");
    output_string(keyword->_rest);
  }
  fputc(']', stdout);
}

void
OutputJSON::output_keywords_table ()
{

  // bool has_rest = false;
  const char *indent = "    ";

  printf("  \"keywords\": [\n");

  int index = 0;
  #if 0
  for (auto head = _head; head; head = head->rest(), ++index) {
    // todo -- duplicates?
    auto keyword = head->first();
    if (index > 0)
      printf (",\n");
    if (index < keyword->_hash_value) {
      output_nulls(indent, keyword->_hash_value - index);
      index = keyword->_hash_value;
    }
    keyword->_final_index = index;

    printf("%s[", indent);
    output_string (keyword->_allchars, keyword->_allchars_length);

    printf(", %d, %d", keyword->_hash_value, keyword->_final_index);


    if (keyword->_rest && *keyword->_rest) {
      printf(", ");
      output_string(keyword->_rest);
    }
    fputc(']', stdout);
    // TODO -- duplicates.
  }
  #else
  for (auto head = _head; head; head = head->rest(), ++index) {
    auto keyword = head->first();
    if (index > 0)
      printf (",\n");
    // keyword->_final_index = index;

    #if 0
    fputs("    [", stdout);
    output_string (keyword->_allchars, keyword->_allchars_length);

    printf(", %d", keyword->_hash_value);

    if (keyword->_rest && *keyword->_rest) {
      printf(", ");
      output_string(keyword->_rest);
    }
    fputc(']', stdout);
    #else
    output_keyword_entry(keyword);
    #endif

    if (keyword->_duplicate_link) {
      for (auto link = keyword->_duplicate_link; link; link = link->_duplicate_link) {
        printf (",\n");
        output_keyword_entry(link);
      }
    }
  }  
  #endif

  printf("\n  ],\n");


}


void
OutputJSON::output_constants ()
{
  printf("  \"total_keywords\": %u,\n", _total_keys);
  printf("  \"min_word_length\": %u,\n", _min_key_len);
  printf("  \"max_word_length\": %u,\n", _max_key_len);
  printf("  \"min_hash_value\": %u,\n", _min_hash_value);
  printf("  \"max_hash_value\": %u,\n", _max_hash_value);
  printf("  \"duplicates\": %s,\n", _total_duplicates ? "true" : "false");
  printf("  \"struct\": %s,\n", _struct_decl ? "true" : "false");
  printf("  \"function_name\": \"%s\",\n", option.get_function_name());
} 

void
OutputJSON::output_lookup_tables ()
{

  const int columns = 10;

  /* Calculate maximum number of digits required for MAX_HASH_VALUE.  */
  int field_width = 2;
  for (int trunc = _max_hash_value; (trunc /= 10) > 0;)
    field_width++;


  printf("  \"asso_values\": [");

  for (unsigned int count = 0; count < _alpha_size; count++)
  {
    if (count > 0)
      printf (",");
    if ((count % columns) == 0)
      printf ("\n    ");
    printf ("%*d", field_width, _asso_values[count]);
  }


  printf("\n  ]\n");

} 


void
OutputJSON::output_hash ()
{

  bool has_alpha_inc = false;

  printf("  \"hash\": {\n");
  printf("    \"include_length\": %s,\n", _hash_includes_len ? "true" : "false");
  printf("    \"positions\": [");
  int count = 0;
  for (auto kIter = _key_positions.iterator(_max_key_len); ; ++count) {
    auto key_pos = kIter.next();
    if (key_pos == PositionIterator::EOS) break;
    if (_alpha_inc[key_pos]) has_alpha_inc = true;
    if (count) printf(", ");
    printf("[%d, %d]", key_pos, key_pos < 0 ? 0 : _alpha_inc[key_pos]);
  }
  printf("]\n"); // positions
  printf("  },\n"); // keys
} 

void
OutputJSON::output ()
{
  compute_min_max ();

  printf("{\n");

  printf("  \"comment\": \"JSON code produced by gperf %s\",\n", version_string);
#if 0
  option.print_options ();
  printf (" */\n");
  if (!option[POSITIONS])
    {
      printf ("/* Computed positions: -k'");
      _key_positions.print();
      printf ("' */\n");
    }
  printf ("\n");
#endif

  output_constants();
  output_hash();
  output_keywords_table();
  output_lookup_tables();


  printf ("}\n");

  fflush (stdout);
}

