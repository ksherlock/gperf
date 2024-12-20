#ifndef output_json_h
#define output_json_h 1

#include "output.h"

class OutputJSON : public Output
{
public:
  /* Constructor. */
  OutputJSON (KeywordExt_List *head,
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
                    const int *asso_values);

  /* Generates the hash function and the key word recognizer function.  */
  void output ();

protected:

void output_hash ();
void output_keywords_table ();
void output_constants ();
void output_lookup_tables ();

};

#endif
