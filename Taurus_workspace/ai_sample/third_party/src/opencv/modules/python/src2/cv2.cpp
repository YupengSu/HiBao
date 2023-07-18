// must be defined before importing numpy headers
// https://numpy.org/doc/1.17/reference/c-api.array.html#importing-the-api
#define PY_ARRAY_UNIQUE_SYMBOL opencv_ARRAY_API

#include "cv2.hpp"

#include "opencv2/opencv_modules.hpp"
#include "opencv2/core.hpp"
#include "opencv2/core/utils/logger.hpp"

#include "pyopencv_generated_include.h"
#include "opencv2/core/types_c.h"


#include "cv2_util.hpp"
#include "cv2_numpy.hpp"
#include "cv2_convert.hpp"
#include "cv2_highgui.hpp"

using namespace cv;

typedef std::vector<uchar> vector_uchar;
typedef std::vector<char> vector_char;
typedef std::vector<int> vector_int;
typedef std::vector<float> vector_float;
typedef std::vector<double> vector_double;
typedef std::vector<size_t> vector_size_t;
typedef std::vector<Point> vector_Point;
typedef std::vector<Point2f> vector_Point2f;
typedef std::vector<Point3f> vector_Point3f;
typedef std::vector<Size> vector_Size;
typedef std::vector<Vec2f> vector_Vec2f;
typedef std::vector<Vec3f> vector_Vec3f;
typedef std::vector<Vec4f> vector_Vec4f;
typedef std::vector<Vec6f> vector_Vec6f;
typedef std::vector<Vec4i> vector_Vec4i;
typedef std::vector<Rect> vector_Rect;
typedef std::vector<Rect2d> vector_Rect2d;
typedef std::vector<RotatedRect> vector_RotatedRect;
typedef std::vector<KeyPoint> vector_KeyPoint;
typedef std::vector<Mat> vector_Mat;
typedef std::vector<std::vector<Mat> > vector_vector_Mat;
typedef std::vector<UMat> vector_UMat;
typedef std::vector<DMatch> vector_DMatch;
typedef std::vector<String> vector_String;
typedef std::vector<std::string> vector_string;
typedef std::vector<Scalar> vector_Scalar;

typedef std::vector<std::vector<char> > vector_vector_char;
typedef std::vector<std::vector<Point> > vector_vector_Point;
typedef std::vector<std::vector<Point2f> > vector_vector_Point2f;
typedef std::vector<std::vector<Point3f> > vector_vector_Point3f;
typedef std::vector<std::vector<DMatch> > vector_vector_DMatch;
typedef std::vector<std::vector<KeyPoint> > vector_vector_KeyPoint;

// enum { ARG_NONE = 0, ARG_MAT = 1, ARG_SCALAR = 2 };


///////////////////////////////////////////////////////////////////////////////////////

static int convert_to_char(PyObject *o, char *dst, const ArgInfo& info)
{
    std::string str;
    if (getUnicodeString(o, str))
    {
        *dst = str[0];
        return 1;
    }
    (*dst) = 0;
    return failmsg("Expected single character string for argument '%s'", info.name);
}

#ifdef __GNUC__
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif


#include "pyopencv_generated_enums.h"

#ifdef CVPY_DYNAMIC_INIT
#define CVPY_TYPE(WNAME, NAME, STORAGE, SNAME, _1, _2) CVPY_TYPE_DECLARE_DYNAMIC(WNAME, NAME, STORAGE, SNAME)
#else
#define CVPY_TYPE(WNAME, NAME, STORAGE, SNAME, _1, _2) CVPY_TYPE_DECLARE(WNAME, NAME, STORAGE, SNAME)
#endif
#include "pyopencv_generated_types.h"
#undef CVPY_TYPE
#include "pyopencv_custom_headers.h"

#include "pyopencv_generated_types_content.h"
#include "pyopencv_generated_funcs.h"

static PyObject* pycvRegisterMatType(PyObject *self, PyObject *value)
{
    CV_LOG_DEBUG(NULL, cv::format("pycvRegisterMatType %p %p\n", self, value));

    if (0 == PyType_Check(value))
    {
        PyErr_SetString(PyExc_TypeError, "Type argument is expected");
        return NULL;
    }

    Py_INCREF(value);
    pyopencv_Mat_TypePtr = (PyTypeObject*)value;

    Py_RETURN_NONE;
}

static PyMethodDef special_methods[] = {
  {"_registerMatType", (PyCFunction)(pycvRegisterMatType), METH_O, "_registerMatType(cv.Mat) -> None (Internal)"},
  {"redirectError", CV_PY_FN_WITH_KW(pycvRedirectError), "redirectError(onError) -> None"},
#ifdef HAVE_OPENCV_HIGHGUI
  {"createTrackbar", (PyCFunction)pycvCreateTrackbar, METH_VARARGS, "createTrackbar(trackbarName, windowName, value, count, onChange) -> None"},
  {"createButton", CV_PY_FN_WITH_KW(pycvCreateButton), "createButton(buttonName, onChange [, userData, buttonType, initialButtonState]) -> None"},
  {"setMouseCallback", CV_PY_FN_WITH_KW(pycvSetMouseCallback), "setMouseCallback(windowName, onMouse [, param]) -> None"},
#endif
#ifdef HAVE_OPENCV_DNN
  {"dnn_registerLayer", CV_PY_FN_WITH_KW(pyopencv_cv_dnn_registerLayer), "registerLayer(type, class) -> None"},
  {"dnn_unregisterLayer", CV_PY_FN_WITH_KW(pyopencv_cv_dnn_unregisterLayer), "unregisterLayer(type) -> None"},
#endif
  {NULL, NULL},
};

/************************************************************************/
/* Module init */

struct ConstDef
{
    const char * name;
    long long val;
};

static void init_submodule(PyObject * root, const char * name, PyMethodDef * methods, ConstDef * consts)
{
  // traverse and create nested submodules
  std::string s = name;
  size_t i = s.find('.');
  while (i < s.length() && i != std::string::npos)
  {
    size_t j = s.find('.', i);
    if (j == std::string::npos)
        j = s.length();
    std::string short_name = s.substr(i, j-i);
    std::string full_name = s.substr(0, j);
    i = j+1;

    PyObject * d = PyModule_GetDict(root);
    PyObject * submod = PyDict_GetItemString(d, short_name.c_str());
    if (submod == NULL)
    {
        submod = PyImport_AddModule(full_name.c_str());
        PyDict_SetItemString(d, short_name.c_str(), submod);
    }

    if (short_name != "")
        root = submod;
  }

  // populate module's dict
  PyObject * d = PyModule_GetDict(root);
  for (PyMethodDef * m = methods; m->ml_name != NULL; ++m)
  {
    PyObject * method_obj = PyCFunction_NewEx(m, NULL, NULL);
    PyDict_SetItemString(d, m->ml_name, method_obj);
    Py_DECREF(method_obj);
  }
  for (ConstDef * c = consts; c->name != NULL; ++c)
  {
    PyDict_SetItemString(d, c->name, PyLong_FromLongLong(c->val));
  }

}

#include "pyopencv_generated_modules_content.h"

static bool init_body(PyObject * m)
{
#define CVPY_MODULE(NAMESTR, NAME) \
    init_submodule(m, MODULESTR NAMESTR, methods_##NAME, consts_##NAME)
    #include "pyopencv_generated_modules.h"
#undef CVPY_MODULE

#ifdef CVPY_DYNAMIC_INIT
#define CVPY_TYPE(WNAME, NAME, _1, _2, BASE, CONSTRUCTOR) CVPY_TYPE_INIT_DYNAMIC(WNAME, NAME, return false, BASE, CONSTRUCTOR)
    PyObject * pyopencv_NoBase_TypePtr = NULL;
#else
#define CVPY_TYPE(WNAME, NAME, _1, _2, BASE, CONSTRUCTOR) CVPY_TYPE_INIT_STATIC(WNAME, NAME, return false, BASE, CONSTRUCTOR)
    PyTypeObject * pyopencv_NoBase_TypePtr = NULL;
#endif
    #include "pyopencv_generated_types.h"
#undef CVPY_TYPE

    PyObject* d = PyModule_GetDict(m);


    PyDict_SetItemString(d, "__version__", PyString_FromString(CV_VERSION));

    PyObject *opencv_error_dict = PyDict_New();
    PyDict_SetItemString(opencv_error_dict, "file", Py_None);
    PyDict_SetItemString(opencv_error_dict, "func", Py_None);
    PyDict_SetItemString(opencv_error_dict, "line", Py_None);
    PyDict_SetItemString(opencv_error_dict, "code", Py_None);
    PyDict_SetItemString(opencv_error_dict, "msg", Py_None);
    PyDict_SetItemString(opencv_error_dict, "err", Py_None);
    opencv_error = PyErr_NewException((char*)MODULESTR".error", NULL, opencv_error_dict);
    Py_DECREF(opencv_error_dict);
    PyDict_SetItemString(d, "error", opencv_error);


#define PUBLISH(I) PyDict_SetItemString(d, #I, PyInt_FromLong(I))
    PUBLISH(CV_8U);
    PUBLISH(CV_8UC1);
    PUBLISH(CV_8UC2);
    PUBLISH(CV_8UC3);
    PUBLISH(CV_8UC4);
    PUBLISH(CV_8S);
    PUBLISH(CV_8SC1);
    PUBLISH(CV_8SC2);
    PUBLISH(CV_8SC3);
    PUBLISH(CV_8SC4);
    PUBLISH(CV_16U);
    PUBLISH(CV_16UC1);
    PUBLISH(CV_16UC2);
    PUBLISH(CV_16UC3);
    PUBLISH(CV_16UC4);
    PUBLISH(CV_16S);
    PUBLISH(CV_16SC1);
    PUBLISH(CV_16SC2);
    PUBLISH(CV_16SC3);
    PUBLISH(CV_16SC4);
    PUBLISH(CV_32S);
    PUBLISH(CV_32SC1);
    PUBLISH(CV_32SC2);
    PUBLISH(CV_32SC3);
    PUBLISH(CV_32SC4);
    PUBLISH(CV_32F);
    PUBLISH(CV_32FC1);
    PUBLISH(CV_32FC2);
    PUBLISH(CV_32FC3);
    PUBLISH(CV_32FC4);
    PUBLISH(CV_64F);
    PUBLISH(CV_64FC1);
    PUBLISH(CV_64FC2);
    PUBLISH(CV_64FC3);
    PUBLISH(CV_64FC4);
#undef PUBLISH

    return true;
}

#if defined(__GNUC__)
#pragma GCC visibility push(default)
#endif

#if defined(CV_PYTHON_3)
// === Python 3

static struct PyModuleDef cv2_moduledef =
{
    PyModuleDef_HEAD_INIT,
    MODULESTR,
    "Python wrapper for OpenCV.",
    -1,     /* size of per-interpreter state of the module,
               or -1 if the module keeps state in global variables. */
    special_methods
};

PyMODINIT_FUNC PyInit_cv2();
PyObject* PyInit_cv2()
{
    import_array(); // from numpy
    PyObject* m = PyModule_Create(&cv2_moduledef);
    if (!init_body(m))
        return NULL;
    return m;
}

#else
// === Python 2
PyMODINIT_FUNC initcv2();
void initcv2()
{
    import_array(); // from numpy
    PyObject* m = Py_InitModule(MODULESTR, special_methods);
    init_body(m);
}

#endif
