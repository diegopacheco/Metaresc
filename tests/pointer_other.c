#include <check.h>
#include <metaresc.h>
#include <regression.h>

TYPEDEF_STRUCT (ld_ptr_t, (const volatile long double *, x))

MR_START_TEST (ld_ptr, "pointer on a long double (type name with spaces)") {
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, ld_ptr_t, NULL);
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, ld_ptr_t, (long double[]){ 0 });
} END_TEST

TYPEDEF_ENUM (_enum_t, ZERO, ONE);
TYPEDEF_ENUM (packed_enum_t, ATTRIBUTES (__attribute__ ((packed, aligned (sizeof (uint16_t))))), PE_ZERO, PE_ONE);
TYPEDEF_STRUCT (enum_ptr_t, (_enum_t *, x));
TYPEDEF_STRUCT (packed_enum_ptr_t, (packed_enum_t *, x));

MR_START_TEST (enum_ptr, "pointer on enum") {
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, enum_ptr_t, NULL);
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, enum_ptr_t, (_enum_t[]){ ZERO });
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, enum_ptr_t, (_enum_t[]){ ONE });
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, packed_enum_ptr_t, NULL);
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, packed_enum_ptr_t, (packed_enum_t[]){ PE_ZERO });
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, packed_enum_ptr_t, (packed_enum_t[]){ PE_ONE });
} END_TEST

static int warnings = 0;
static void
msg_handler (const char * file_name, const char * func_name, int line, mr_log_level_t log_level, mr_message_id_t message_id, va_list args)
{
  if ((MR_MESSAGE_SAVE_ENUM == message_id) || (MR_MESSAGE_SAVE_BITMASK == message_id))
    ++warnings;
}

MR_START_TEST (invalid_enum_ptr, "pointer on invalid enum") {
  int checked = 0;
  mr_msg_handler_t save_msg_handler = mr_conf.msg_handler;

#define CMP_ENUMS(TYPE, X, Y, ...) ({ ++checked; (*((X)->x) != *((Y)->x));})

  mr_conf.msg_handler = msg_handler;
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, enum_ptr_t, (_enum_t[]){ -1 }, CMP_ENUMS);
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, packed_enum_ptr_t, (packed_enum_t[]){ -1 }, CMP_ENUMS);
  mr_conf.msg_handler = save_msg_handler;

  ck_assert_msg ((checked == warnings), "Save/load of ivnalid enum value didn't produced mathced number of warnings (%d != %d)", checked, warnings);
} END_TEST

TYPEDEF_STRUCT (char_ptr_t, POINTER (char, x));

MR_START_TEST (char_ptr, "pointer on char") {
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, char_ptr_t, NULL);
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, char_ptr_t, "x");
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, char_ptr_t, (char[]){ 0 });
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, char_ptr_t, (char[]){ 255 });
} END_TEST

TYPEDEF_STRUCT (charp_ptr_t, (char**, x));
TYPEDEF_STRUCT (string_ptr_t, (string_t*, x));

MR_START_TEST (string_ptr, "pointer on string") {
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, charp_ptr_t, NULL);
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, charp_ptr_t, (char*[]){ "x" });
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, string_ptr_t, NULL);
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, string_ptr_t, (string_t[]){ "x" });
} END_TEST

TYPEDEF_STRUCT (struct_ptr_t, (enum_ptr_t*, x));

MR_START_TEST (struct_ptr, "pointer on struct") {
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, struct_ptr_t, NULL);
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, struct_ptr_t, (enum_ptr_t[]){ { NULL } });
  ALL_METHODS (ASSERT_SAVE_LOAD_TYPE, struct_ptr_t, (enum_ptr_t[]){ { (_enum_t[]){ ZERO } } });
} END_TEST

TYPEDEF_STRUCT (self_ref_ptr_t, (_enum_t *, x), (_enum_t, y));

MR_START_TEST (self_ref_ptr, "self referenced pointer") {
  self_ref_ptr_t x = { (_enum_t[]){ ZERO }, ONE };
  ALL_METHODS (ASSERT_SAVE_LOAD, self_ref_ptr_t, &x);
  x.x = &x.y;
  ALL_METHODS (ASSERT_SAVE_LOAD, self_ref_ptr_t, &x);
} END_TEST

TYPEDEF_STRUCT (self_ref_string_t,
		CHAR_ARRAY (char, x, [sizeof ("x")]),
		string_t y,
		(string_t *, z),
		mr_ptr_t v);

MR_START_TEST (self_ref_string, "self referenced strings") {
  self_ref_string_t x = { "x", "y", (string_t[]){ "z" }};
  x.v.ptr = &x.z; /* resolved not-NULL void pointer */
  ALL_METHODS (ASSERT_SAVE_LOAD, self_ref_string_t, &x);
  x.z = &x.y;
  ALL_METHODS (ASSERT_SAVE_LOAD, self_ref_string_t, &x);
  x.y = x.x;
  ALL_METHODS (ASSERT_SAVE_LOAD, self_ref_string_t, &x);
  x.v.ptr = "z"; /* unresolved not-NULL void pointer */
  ALL_METHODS (ASSERT_SAVE_LOAD, self_ref_string_t, &x);
} END_TEST

TYPEDEF_STRUCT (rtfr_struct_t,
		int x,
		int y);

TYPEDEF_STRUCT (resolve_typed_forward_ref_t,
		(rtfr_struct_t *, x),
		(int *, y) /* pointers are loaded via stack, so one level ponters will be loaded in a reverse order */
		);

MR_START_TEST (resolve_typed_forward_ref, "test of forvard reference resolution") {
  resolve_typed_forward_ref_t x = { (rtfr_struct_t[]){ {0, 1} } };
  x.y = &x.x->y;
  ALL_METHODS (ASSERT_SAVE_LOAD, resolve_typed_forward_ref_t, &x);
} END_TEST

TYPEDEF_UNION (int_float_t,
	       int _int,
	       float _float,
	       )

TYPEDEF_STRUCT (typed_union_t,
		string_t type,
		(int_float_t, uif, , "type"),
		)

TYPEDEF_STRUCT (union_resolution_t,
		(typed_union_t *, typed_union),
		string_t type,
		(int_float_t *, uifp, , "type"),
		)

MR_START_TEST (union_resolution_correctness, "test correctness of union resolution") {
  union_resolution_t union_resolution = {
    .type = "_float",
    .typed_union = (typed_union_t[]){
      {
	.type = "_int",
	.uif = { ._int = 1234567890, },
      },
    },
  };
  union_resolution.uifp = &union_resolution.typed_union->uif;
  
  mr_ra_ptrdes_t ptrs = MR_SAVE (union_resolution_t, &union_resolution);
  int i;
  bool union_resolved_correctly = false;
  if (ptrs.ra != NULL)
    {
      for (i = ptrs.size / sizeof (ptrs.ra[0]) - 1; i >= 0; --i)
	{
	  if ((ptrs.ra[i].fd.name.str != NULL) &&
	      (0 == strcmp (ptrs.ra[i].fd.name.str, "uif")) &&
	      (ptrs.ra[i].first_child >= 0) &&
	      (ptrs.ra[ptrs.ra[i].first_child].fd.name.str != NULL) &&
	      (0 == strcmp (ptrs.ra[ptrs.ra[i].first_child].fd.name.str, union_resolution.typed_union->type)))
	    {
	      union_resolved_correctly = true;
	      break;
	    }
	}
      MR_FREE (ptrs.ra);
    }
  ck_assert_msg (union_resolved_correctly, "Union resolved incorrectly");
} END_TEST

TYPEDEF_STRUCT (linked_list_ptr_t,
		(struct linked_list_t *, ptr));

TYPEDEF_STRUCT (linked_list_t,
		(linked_list_ptr_t, next));

TYPEDEF_STRUCT (root_struct_t,
		(linked_list_ptr_t *, ll_ptr));

MR_START_TEST (backward_ref_is_a_field, "Saved pointer is a field in struct") {
  linked_list_t ll;
  root_struct_t root_struct;
  ll.next.ptr = &ll;
  root_struct.ll_ptr = &ll.next;

  mr_ra_ptrdes_t ptrs = MR_SAVE (root_struct_t, &root_struct);
  int i;
  bool pointer_resolved_correctly = true;
  if (ptrs.ra != NULL)
    {
      for (i = ptrs.size / sizeof (ptrs.ra[0]) - 1; i >= 0; --i)
	{
	  int ref_idx = ptrs.ra[i].ref_idx;
	  if ((ref_idx >= 0) &&
	      (ptrs.ra[ref_idx].idx < 0))
	    {
	      pointer_resolved_correctly = false;
	      break;
	    }
	}
      MR_FREE (ptrs.ra);
    }
  ck_assert_msg (pointer_resolved_correctly, "Pointer resolved incorrectly");
} END_TEST

TYPEDEF_STRUCT (two_dynamic_arrays_t,
		(int *, da1, , "dynamic array one", { .offset = offsetof (two_dynamic_arrays_t, size1), }, "offset"),
		(ssize_t, size1),
		(int *, da2, , "dynamic array two", { .offset = offsetof (two_dynamic_arrays_t, size2), }, "offset"),
		(ssize_t, size2),
		);

MR_START_TEST (tda_same_ptr_and_size, "Two dynamic arrays with same pointers and size") {
  two_dynamic_arrays_t tda;
  int array[2] = { 1, 2, };

  tda.da1 = array;
  tda.size1 = sizeof (array);
  tda.da2 = array;
  tda.size2 = sizeof (array);
  
  mr_ra_ptrdes_t ptrs = MR_SAVE (two_dynamic_arrays_t, &tda);
  int i;
  bool pointer_resolved_correctly = true;
  if (ptrs.ra != NULL)
    {
      for (i = ptrs.size / sizeof (ptrs.ra[0]) - 1; i >= 0; --i)
	{
	  int ref_idx = ptrs.ra[i].ref_idx;
	  if ((ref_idx >= 0) &&
	      (ptrs.ra[ref_idx].prev >= 0))
	    {
	      pointer_resolved_correctly = false;
	      break;
	    }
	}
      MR_FREE (ptrs.ra);
    }
  ck_assert_msg (pointer_resolved_correctly, "Pointer resolved incorrectly");
} END_TEST

MR_START_TEST (tda_same_ptr_and_bigger, "Two dynamic arrays with same pointers and one is bigger") {
  two_dynamic_arrays_t tda;
  int array[2] = { 1, 2, };

  tda.da1 = array;
  tda.size1 = sizeof (array);
  tda.da2 = array;
  tda.size2 = sizeof (array[0]);
  
  mr_ra_ptrdes_t ptrs = MR_SAVE (two_dynamic_arrays_t, &tda);
  int i;
  bool pointer_resolved_correctly = true;
  if (ptrs.ra != NULL)
    {
      for (i = ptrs.size / sizeof (ptrs.ra[0]) - 1; i >= 0; --i)
	{
	  int ref_idx = ptrs.ra[i].ref_idx;
	  if ((ref_idx >= 0) &&
	      (ptrs.ra[ref_idx].next < 0))
	    {
	      pointer_resolved_correctly = false;
	      break;
	    }
	}
      MR_FREE (ptrs.ra);
    }
  ck_assert_msg (pointer_resolved_correctly, "Pointer resolved incorrectly");
} END_TEST

MR_START_TEST (tda_overlapping_1, "Two overlaping dynamic arrays. Lower pointer saved first") {
  two_dynamic_arrays_t tda;
  int array[3] = { 1, 2, 3, };

  tda.da1 = &array[1];
  tda.size1 = 2 * sizeof (array[0]);
  tda.da2 = &array[0];
  tda.size2 = 2 * sizeof (array[0]);
  
  mr_ra_ptrdes_t ptrs = MR_SAVE (two_dynamic_arrays_t, &tda);
  int i;
  bool pointer_resolved_correctly = false;
  if (ptrs.ra != NULL)
    {
      for (i = ptrs.size / sizeof (ptrs.ra[0]) - 1; i >= 0; --i)
	{
	  int ref_idx = ptrs.ra[i].ref_idx;
	  if (ref_idx >= 0)
	    {
	      pointer_resolved_correctly = true;
	      break;
	    }
	}
      MR_FREE (ptrs.ra);
    }
  ck_assert_msg (pointer_resolved_correctly, "Pointer resolved incorrectly");
} END_TEST

MR_START_TEST (tda_overlapping_2, "Two overlaping dynamic arrays. Lower pointer saved second") {
  two_dynamic_arrays_t tda;
  int array[3] = { 1, 2, 3, };

  tda.da1 = &array[0];
  tda.size1 = 2 * sizeof (array[0]);
  tda.da2 = &array[1];
  tda.size2 = 2 * sizeof (array[0]);
  
  mr_ra_ptrdes_t ptrs = MR_SAVE (two_dynamic_arrays_t, &tda);
  int i;
  bool pointer_resolved_correctly = false;
  if (ptrs.ra != NULL)
    {
      for (i = ptrs.size / sizeof (ptrs.ra[0]) - 1; i >= 0; --i)
	{
	  int ref_idx = ptrs.ra[i].ref_idx;
	  if (ref_idx >= 0)
	    {
	      pointer_resolved_correctly = true;
	      break;
	    }
	}
      MR_FREE (ptrs.ra);
    }
  ck_assert_msg (pointer_resolved_correctly, "Pointer resolved incorrectly");
} END_TEST


TYPEDEF_STRUCT (int_array_t,
		(int, array, [3]),
		);

TYPEDEF_STRUCT (pointer_to_array_t,
		(int_array_t *, array_ptr),
		(int *, da, , "dynamic array", { .offset = offsetof (pointer_to_array_t, size), }, "offset"),
		(ssize_t, size),
		);

MR_START_TEST (pointer_to_array, "Pointer into the middle of static array loaded first") {
  pointer_to_array_t pointer_to_array;
  int_array_t array = { { 1, 2, 3 }, };
  pointer_to_array.array_ptr = &array;
  pointer_to_array.da = &array.array[1];
  pointer_to_array.size = 2 * sizeof (pointer_to_array.da[0]);
  
  mr_ra_ptrdes_t ptrs = MR_SAVE (pointer_to_array_t, &pointer_to_array);
  int i;
  bool pointer_resolved_correctly = false;
  if (ptrs.ra != NULL)
    {
      for (i = ptrs.size / sizeof (ptrs.ra[0]) - 1; i >= 0; --i)
	{
	  int ref_idx = ptrs.ra[i].ref_idx;
	  if ((ref_idx >= 0) && (ptrs.ra[ref_idx].prev >= 0))
	    {
	      pointer_resolved_correctly = true;
	      break;
	    }
	}
      MR_FREE (ptrs.ra);
    }
  ck_assert_msg (pointer_resolved_correctly, "Pointer resolved incorrectly");
} END_TEST

MAIN ();
