%{
  %token T_and          "and"
  %token T_array        "array"
  %token T_begin        "begin"
  %token T_boolean      "boolean"
  %token T_char         "char"
  %token T_dispose      "dispose"
  %token T_div          "div"
  %token T_do           "do"
  %token T_else         "else"
  %token T_end          "end"
  %token T_false        "false"
  %token T_forward      "forward"
  %token T_function     "function"
  %token T_goto         "goto"
  %token T_if           "if"
  %token T_integer      "integer"
  %token T_label        "label"
  %token T_mod          "mod"
  %token T_new          "new"
  %token T_nil          "nil"
  %token T_not          "not"
  %token T_of           "of"
  %token T_or           "or"
  %token T_procedure    "procedure"
  %token T_program      "program"
  %token T_real         "real"
  %token T_result       "result"
  %token T_return       "return"
  %token T_then         "then"
  %token T_true         "true"
  %token T_var          "var"
  %token T_while        "while"


  %token T_op_eq          "="
  %token T_op_g           ">"
  %token T_op_l           "<"
  %token T_op_neq         "<>"
  %token T_op_leq         "<="
  %token T_op_geq         ">="
  %token T_op_p           "+"
  %token T_op_m           "-"
  %token T_op_mul         "*"
  %token T_op_d           "/"
  %token T_op_point       "^"
  %token T_op_addr        "@"

  %token T_op_assign      ":="
  %token T_op_semicol     ";"
  %token T_op_dot         "."
  %token T_op_lpar        "("
  %token T_op_rpar        ")"
  %token T_op_col         ":"
  %token T_op_com         ","
  %token T_op_lbr         "["
  %token T_op_rbr         "]"
  %token T_int_const
  %token T_real_const
  %token T_const_char
  %token T_const_string
  %token T_id

  /*operators*/
  %left "or"
  %left "and"
  %left "not"
  %nonassoc '=' "<>" '>' '<' "<=" ">="
  %left '^'
  %left '+' '-'
  %left '*' '/' "mod" "div"
%}

%%

%%
