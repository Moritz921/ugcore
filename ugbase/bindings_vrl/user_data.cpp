#include "../ug_bridge/ug_bridge.h"
#include "type_converter.h"
#include "common/common.h"
#include "../lib_discretization/spatial_discretization/ip_data/user_data.h"
#include "../ug_script/ug_script.h"
#include "bindings_vrl.h"
#include "type_converter.h"
#include "threading.h"
#include <iostream>
#include <sstream>

#include "user_data.h"

namespace ug {
namespace vrl {

/// Groovy/Java Converter to read/write data from/to JVM
//template <std::size_t dim>
//struct VectorConverter;

//template <>
//struct vrl_traits<number> {
//
//	static void write(JNIEnv *env, jdouble& d, const number c) {
//		d = c;
//	}
//
//	static void read(JNIEnv *env, jdouble d, number & c) {
//		c = d;
//	}
//
//	static const int size = 1;
//};


//template <std::size_t dim>
//struct vrl_traits< MathMatrix<dim, dim> > {
//
//	static void write(lua_State* L, const MathMatrix<dim, dim>& D) {
//		for (size_t i = 0; i < dim; ++i) {
//			for (size_t j = 0; j < dim; ++j) {
//				lua_pushnumber(L, D[i][j]);
//			}
//		}
//
//	}
//
//	static void read(lua_State* L, MathMatrix<dim, dim>& D) {
//		int counter = -1;
//		for (size_t i = 0; i < dim; ++i) {
//			for (size_t j = 0; j < dim; ++j) {
//				D[dim - 1 - j][dim - 1 - i] = luaL_checknumber(L, counter--);
//			}
//		}
//	}
//
//	static const int size = dim*dim;
//};

template <std::size_t dim>
struct VectorConverter {

	static jdoubleArray toJava(JNIEnv *env,
			const MathVector<dim>& x, const number time = 0.0) {
		jdoubleArray result = NULL;
		result = env->NewDoubleArray(dim + 1);
		number elements[dim + 1];
		//		jdouble *elements = env->GetDoubleArrayElements(result, NULL);
		for (size_t i = 0; i < dim; i++) {
			elements[i] = x[i];
		}
		elements[dim] = time;

		//		env->ReleaseDoubleArrayElements(result, elements, 0);
		env->SetDoubleArrayRegion(result, 0, dim + 1, elements);
		return result;
	}

	static jdoubleArray toJava(JNIEnv *env,
			const MathVector<dim>& x) {
		jdoubleArray result = NULL;
		result = env->NewDoubleArray(dim);
		number elements[dim + 1];
		//		jdouble *elements = env->GetDoubleArrayElements(result, NULL);
		for (size_t i = 0; i < dim; i++) {
			elements[i] = x[i];
		}

		//		env->ReleaseDoubleArrayElements(result, elements, 0);
		env->SetDoubleArrayRegion(result, 0, dim, elements);
		return result;
	}

	static void toC(JNIEnv *env, jdoubleArray& array, MathVector<dim>& x) {
		jdouble elements[dim];
		env->GetDoubleArrayRegion(array, 0, dim, elements);

		jint arrayLength = env->GetArrayLength(array);

		if (arrayLength != dim) {
			UG_LOG(RED_BEGIN << "VRLUserData: dimensions do not match! Required: "
					<< dim << ", returned: " << arrayLength << COLOR_END << std::endl);
		}

		for (size_t i = 0; i < dim; ++i) {
			x[i] = elements[i];
		}
	}

	static const int size = dim;
};

jdouble boundaryReturnData2Double(JNIEnv *env, jobject obj) {
	jdouble result = 0;

	jclass cls = env->FindClass("edu/gcsc/vrl/ug4/Boundary");

	if (env->ExceptionCheck()) {
		env->ExceptionDescribe();
	}

	jmethodID method = env->GetMethodID(cls, "getValue", "()D");

	if (env->ExceptionCheck()) {
		env->ExceptionDescribe();
	}

	result = env->CallDoubleMethod(obj, method);

	return result;
}

jdouble boundaryReturnData2Boolean(JNIEnv *env, jobject obj) {
	jdouble result = 0;

	jclass cls = env->FindClass("edu/gcsc/vrl/ug4/Boundary");

	if (env->ExceptionCheck()) {
		env->ExceptionDescribe();
	}

	jmethodID method = env->GetMethodID(cls, "getBndBool", "()Z");

	if (env->ExceptionCheck()) {
		env->ExceptionDescribe();
	}

	result = env->CallBooleanMethod(obj, method);

	return result;
}

jobject compileUserDataString(JNIEnv *env, const char* s, unsigned int returnValueDim) {
	jclass cls = env->FindClass(
			"edu/gcsc/vrl/ug4/UserDataCompiler");

	if (env->ExceptionCheck()) {
		env->ExceptionDescribe();
	}

	jmethodID runMethod = env->GetStaticMethodID(
			cls, "compile",
			"(Ljava/lang/String;I)Ljava/lang/Object;");

	if (env->ExceptionCheck()) {
		env->ExceptionDescribe();
	}

	return env->CallStaticObjectMethod(cls, runMethod, stringC2J(env, s),
			returnValueDim);
}

jclass getUserDataClass(JNIEnv *env) {
	jclass result = env->FindClass(
			"edu/gcsc/vrl/ug4/UserData");

	if (env->ExceptionCheck()) {
		env->ExceptionDescribe();
	}

	return result;
}

jmethodID getUserDataRunMethod(JNIEnv *env, jclass cls, int dim, const char* signature) {

	std::stringstream mName;

	mName << "run" << dim;

	jmethodID result = env->GetMethodID(cls, mName.str().c_str(), signature);

	if (!checkException(env)) {

		UG_LOG("[VRL-Bindings] Error:"
				<< " cannot find userdata method."
				<< " Please check your implementation!" << std::endl);
	}
	return result;
}

jobject compileBoundaryUserDataString(JNIEnv *env, const char* s) {
	jclass cls = env->FindClass(
			"edu/gcsc/vrl/ug4/UserDataCompiler");

	if (env->ExceptionCheck()) {
		env->ExceptionDescribe();
	}

	jmethodID runMethod = env->GetStaticMethodID(
			cls, "compile",
			"(Ljava/lang/String;I)Ljava/lang/Object;");

	if (env->ExceptionCheck()) {
		env->ExceptionDescribe();
	}

	return env->CallStaticObjectMethod(cls, runMethod, stringC2J(env, s));
}

jclass getBoundaryUserDataClass(JNIEnv *env) {
	jclass result = env->FindClass(
			"edu/gcsc/vrl/ug4/BoundaryUserData");

	if (env->ExceptionCheck()) {
		env->ExceptionDescribe();
	}

	return result;
}

jmethodID getBoundaryUserDataRunMethod(JNIEnv *env, jclass cls) {

	std::string signature =
			"([D)Ledu/gcsc/vrl/ug4/Boundary;";

	std::stringstream mName;

	mName << "run";

	jmethodID result = env->GetMethodID(
			cls, mName.str().c_str(), signature.c_str());

	if (!checkException(env)) {

		UG_LOG("[VRL-Bindings] Error:"
				<< " cannot find boundary-userdata method."
				<< " Please check your implementation!" << std::endl);
	}
	return result;
}

template <int dim>
class UserNumber : public IUserData<number, dim> {
public:

	///	Base class type
	typedef IUserData<number, dim> base_type;

	using base_type::num_series;
	using base_type::num_ip;
	using base_type::ip;
	using base_type::time;
	using base_type::value;


	//	Functor Type
	typedef typename base_type::functor_type functor_type;

	//	return functor

	virtual functor_type get_functor() const {
		return *this;
	}

public:

	UserNumber() {
		javaVM = getJavaVM();
		expression = "";
		returnValueDim = 0;

		std::stringstream stream;
		stream << "<font color=\"red\">VRLUserNumber"
				<< dim << "D: invokation error:</font>";
		invocationErrorMsg = stream.str();
		initialized = false;
	}

	void set_vrl_callback(const char* expression) {
		this->expression = expression;

		JNIEnv* localEnv = threading::getEnv(getJavaVM());

		userDataObject = compileUserDataString(localEnv, expression, returnValueDim);
		userDataClass = getUserDataClass(localEnv);
		runMethod = getUserDataRunMethod(
				localEnv, userDataClass, returnValueDim, "([D)D");

		// create thread-safe references
		// (GC won't deallocate them until manual deletion request)
		userDataObject = localEnv->NewGlobalRef(userDataObject);
		userDataClass = (jclass) localEnv->NewGlobalRef((jobject) userDataClass);

		initialized = true;
	}

	///	evaluates the data at a given point and time

	void operator() (number& c, const MathVector<dim>& x, number time = 0.0) {

		JNIEnv* localEnv = threading::getEnv(getJavaVM());

		//		// TODO this should be cached!!!
		//		// <BEGIN>
		//		userDataObject = compileUserDataString(localEnv, expression.c_str(), returnValueDim);
		//		userDataClass = getUserDataClass(localEnv);
		//		runMethod = getUserDataRunMethod(
		//				localEnv, userDataClass, returnValueDim, "([D)D");
		//		// <END>

		jdoubleArray params = VectorConverter<dim>::toJava(localEnv, x, time);

		if (runMethod != NULL) {
			c = localEnv->CallDoubleMethod(
					userDataObject,
					runMethod,
					params);

			if (checkException(localEnv, invocationErrorMsg)) {
				// currently nothing to do, only necessary for arrays
			}
		}
	}

	///	implement as a IPData

	virtual void compute(bool computeDeriv = false) {
		for (size_t s = 0; s < num_series(); ++s)
			for (size_t i = 0; i < num_ip(s); ++i) {
				this->operator()(value(s, i),
						ip(s, i),
						time());
			}
	}

	~UserNumber() {
		// deleting thread-safe global references
//		if (initialized) {
//			JNIEnv* localEnv = threading::getEnv(getJavaVM());
//			localEnv->DeleteGlobalRef(userDataObject);
//			localEnv->DeleteGlobalRef((jobject) userDataClass);
//		}
	}

private:
	std::string expression;
	JavaVM* javaVM;
	jobject userDataObject;
	jclass userDataClass;
	jmethodID runMethod;
	unsigned int returnValueDim;
	std::string invocationErrorMsg;
	bool initialized;
};

template <int dim>
class UserVector : public IUserData<MathVector<dim>, dim> {
public:

	///	Base class type
	typedef IUserData<MathVector<dim>, dim> base_type;

	using base_type::num_series;
	using base_type::num_ip;
	using base_type::ip;
	using base_type::time;
	using base_type::value;


	//	Functor Type
	typedef typename base_type::functor_type functor_type;

	//	return functor

	virtual functor_type get_functor() const {
		return *this;
	}

public:

	UserVector() {
		javaVM = getJavaVM();
		expression = "";
		returnValueDim = 1;

		std::stringstream stream;
		stream << "<font color=\"red\">VRLUserNumber"
				<< dim << "D: invokation error:</font>";
		invocationErrorMsg = stream.str();

		initialized = false;
	}

	void set_vrl_callback(const char* expression) {
		this->expression = expression;

		JNIEnv* localEnv = threading::getEnv(getJavaVM());

		userDataObject = compileUserDataString(localEnv, expression, returnValueDim);
		userDataClass = getUserDataClass(localEnv);
		runMethod = getUserDataRunMethod(
				localEnv, userDataClass, returnValueDim, "([D)[D");

		if (checkException(localEnv, "creatinerror")) {
			UG_LOG(">> HERE 0\n");
		}

		// create thread-safe references
		// (GC won't deallocate them until manual deletion request)
		userDataObject = localEnv->NewGlobalRef(userDataObject);
		userDataClass = (jclass) localEnv->NewGlobalRef((jobject) userDataClass);

		if (checkException(localEnv, "globalref error")) {
			UG_LOG(">> HERE 1\n");
		}

		initialized = true;
	}

	///	evaluates the data at a given point and time

	void operator() (MathVector<dim>& c, const MathVector<dim>& x, number time = 0.0) {

		JNIEnv* localEnv = threading::getEnv(getJavaVM());

		//		// TODO this should be cached!!!
		//		// <BEGIN>
		//		userDataObject = compileUserDataString(localEnv, expression.c_str(), returnValueDim);
		//		userDataClass = getUserDataClass(localEnv);
		//		runMethod = getUserDataRunMethod(
		//				localEnv, userDataClass, returnValueDim, "([D)[D");
		//		// <END>

		jdoubleArray params = VectorConverter<dim>::toJava(localEnv, x, time);

		if (checkException(localEnv, "paramconverter error")) {
			UG_LOG(">> HERE 2\n");
		}

		if (runMethod != NULL) {
			jdoubleArray result = (jdoubleArray) localEnv->CallObjectMethod(
					userDataObject,
					runMethod,
					params);

			if (checkException(localEnv, invocationErrorMsg)) {
				VectorConverter<dim>::toC(localEnv, result, c);
			}
		}

		if (checkException(localEnv, "???")) {
			UG_LOG(">> HERE 3\n");
		}
	}

	///	implement as a IPData

	virtual void compute(bool computeDeriv = false) {
		for (size_t s = 0; s < num_series(); ++s)
			for (size_t i = 0; i < num_ip(s); ++i) {
				this->operator()(value(s, i),
						ip(s, i),
						time());
			}
	}

	~UserVector() {

		// deleting thread-safe global references
//		if (initialized) {
//			JNIEnv* localEnv = threading::getEnv(getJavaVM());
//			localEnv->DeleteGlobalRef(userDataObject);
//			localEnv->DeleteGlobalRef((jobject) userDataClass);
//		}
	}

private:
	std::string expression;
	JavaVM* javaVM;
	jobject userDataObject;
	jclass userDataClass;
	jmethodID runMethod;
	unsigned int returnValueDim;
	std::string invocationErrorMsg;
	bool initialized;
};

template <int dim>
class BoundaryNumber : public IBoundaryData<number, dim> {
public:

	///	Base class type
	typedef IBoundaryData<number, dim> base_type;

	//	Functor Type
	typedef typename base_type::functor_type functor_type;

	//	return functor

	virtual functor_type get_functor() const {
		return *this;
	}

public:

	BoundaryNumber() {
		javaVM = getJavaVM();
		expression = "";

		std::stringstream stream;
		stream << "<font color=\"red\">VRLUserNumber"
				<< dim << "D: invokation error:</font>";
		invocationErrorMsg = stream.str();

		initialized = false;
	}

	void set_vrl_callback(const char* expression) {
		this->expression = expression;

		JNIEnv* localEnv = threading::getEnv(getJavaVM());

		userDataObject =
				compileBoundaryUserDataString(localEnv, expression);
		userDataClass = getBoundaryUserDataClass(localEnv);
		runMethod = getBoundaryUserDataRunMethod(localEnv, userDataClass);

		// create thread-safe references 
		// (GC won't deallocate them until manual deletion request)
		userDataObject = localEnv->NewGlobalRef(userDataObject);
		userDataClass = (jclass) localEnv->NewGlobalRef((jobject) userDataClass);

		initialized = true;
	}

	///	evaluates the data at a given point and time

	bool operator() (number& c, const MathVector<dim>& x, number time = 0.0) {

		JNIEnv* localEnv = threading::getEnv(getJavaVM());

		bool result = false;

		jdoubleArray params = VectorConverter<dim>::toJava(localEnv, x, time);

		if (runMethod != NULL) {
			jobject bndResult = (jdoubleArray) localEnv->CallObjectMethod(
					userDataObject,
					runMethod,
					params);

			if (checkException(localEnv, invocationErrorMsg)) {
				result = boundaryReturnData2Boolean(localEnv, bndResult);
				c = boundaryReturnData2Double(localEnv, bndResult);
			}
		}

		return result;
	}

	~BoundaryNumber() {
		// deleting thread-safe global references
//		if (initialized) {
//			JNIEnv* localEnv = threading::getEnv(getJavaVM());
//			localEnv->DeleteGlobalRef(userDataObject);
//			localEnv->DeleteGlobalRef((jobject) userDataClass);
//		}
	}

private:
	std::string expression;
	JavaVM* javaVM;
	jobject userDataObject;
	jclass userDataClass;
	jmethodID runMethod;
	std::string invocationErrorMsg;
	bool initialized;
};

class PrintUserNumber2d {
protected:
	typedef IUserData<number, 2 > ::functor_type NumberFunctor;

public:

	void set_user_number(IUserData<number, 2 > & user) {
		m_Number = user.get_functor();
	}

	number print(number x, number y) {
		MathVector < 2 > v(x, y);
		number time = 0.0;
		number ret;

		if (m_Number)
			m_Number(ret, v, time);
		else {
			UG_LOG("Functor not set. \n");
			ret = -1;
		}

		return ret;
	}

private:
	NumberFunctor m_Number;
};

class PrintUserVector2d {
protected:
	typedef IUserData<MathVector < 2 >, 2 > ::functor_type NumberFunctor;

public:

	void set_user_vector(IUserData<MathVector < 2 >, 2 > & user) {
		m_Number = user.get_functor();
	}

	void print(number x, number y) {
		MathVector < 2 > v(x, y);
		number time = 0.0;
		MathVector < 2 > ret;

		if (m_Number)
			m_Number(ret, v, time);
		else {
			UG_LOG("Functor not set. \n");
			ret = -1;
		}

		for (size_t i = 0; i < ret.size(); i++) {
			UG_LOG("VECTOR[" << i << "]=" << ret[i] << std::endl);
		}
	}

private:
	NumberFunctor m_Number;
};

class PrintBoundaryNumber2d {
protected:
	typedef IBoundaryData<number, 2 > ::functor_type NumberFunctor;

public:

	void set_user_number(IBoundaryData<number, 2 > & user) {
		m_Number = user.get_functor();
	}

	std::string print(number x, number y) {
		MathVector < 2 > v(x, y);
		number time = 0.0;
		number ret;
		bool bndResult = false;

		if (m_Number)
			bndResult = m_Number(ret, v, time);
		else {
			UG_LOG("Functor not set. \n");
			ret = -1;
		}

		std::stringstream stream;

		stream << "[" << bndResult << "," << ret << "]";

		return stream.str();
	}

private:
	NumberFunctor m_Number;
};

template <int dim>
void RegisterUserData(ug::bridge::Registry& reg,
		std::vector<const char*> paramNames, const char* parentGroup) {
	std::string grp = std::string(parentGroup);

	////	Base class
	//	{
	//		stringstream ss; ss << "IUserNumberProvider" << dim << "d";
	//		reg.add_class_<IUserNumberProvider<dim> >(ss.str().c_str(), grp.c_str());
	//	}

	////	Functor
	//	{
	//	//	ConstUserNumber
	//		{
	//			typedef ConstUserNumber<dim> T;
	//			stringstream ss; ss << "ConstUserNumber" << dim << "d";
	//			reg.add_class_<T, IUserNumberProvider<dim> >(ss.str().c_str(), grp.c_str())
	//				.add_constructor()
	//				.add_method("set", &T::set)
	//				.add_method("print", &T::print);
	//		}
	//
	//	}


	//	VRLUserNumber
	{
		typedef UserNumber<dim> T;
		std::stringstream className;
		className << "VRLUserNumber" << dim << "d";
		std::stringstream options;
		options << "Input:|user-data|dim=" << 0 << ";" << "params=[";

		for (size_t i = 0; i < paramNames.size(); i++) {
			if (i > 0) {
				options << ",";
			}
			options << "\"" << paramNames[i] << "\"";
		}

		options << "];";

		reg.add_class_<T, IUserData<number, dim> >(
				className.str().c_str(), grp.c_str())
				.add_constructor()
				.add_method("userNumber", &T::set_vrl_callback, "",
				options.str().c_str());
	}

	//	VRLUserVector
	{
		typedef UserVector<dim> T;
		std::stringstream className;
		className << "VRLUserVector" << dim << "d";
		std::stringstream options;
		options << "Input:|user-data|dim=" << 1 << ";" << "params=[";

		for (size_t i = 0; i < paramNames.size(); i++) {
			if (i > 0) {
				options << ",";
			}
			options << "\"" << paramNames[i] << "\"";
		}

		options << "];";

		reg.add_class_<T, IUserData<MathVector<dim>, dim> >(
				className.str().c_str(), grp.c_str())
				.add_constructor()
				.add_method("userVector", &T::set_vrl_callback, "",
				options.str().c_str());
	}

	//	VRLBoundaryUserVector
	{
		typedef BoundaryNumber<dim> T;
		std::stringstream className;
		className << "VRLBoundaryNumber" << dim << "d";
		std::stringstream options;
		options << "Input:|boundary-user-data|params=[";

		for (size_t i = 0; i < paramNames.size(); i++) {
			if (i > 0) {
				options << ",";
			}
			options << "\"" << paramNames[i] << "\"";
		}

		options << "];";

		reg.add_class_<T, IBoundaryData<number, dim> >(
				className.str().c_str(), grp.c_str())
				.add_constructor()
				.add_method("boundaryNumber", &T::set_vrl_callback, "",
				options.str().c_str());
	}

}

void RegisterUserData(ug::bridge::Registry& reg, const char* parentGroup) {
	std::vector<const char*> paramNames;
#ifdef UG_DIM_1
	paramNames.push_back("x");
	paramNames.push_back("t");
	ug::vrl::RegisterUserData < 1 > (reg, paramNames, parentGroup);
#endif
#ifdef UG_DIM_2
	paramNames.clear();
	paramNames.push_back("x");
	paramNames.push_back("y");
	paramNames.push_back("t");
	ug::vrl::RegisterUserData < 2 > (reg, paramNames, parentGroup);

	typedef PrintUserNumber2d T;
	std::stringstream ss;
	ss << "PrintUserNumber2d";
	reg.add_class_<T > (ss.str().c_str(), parentGroup)
			.add_constructor()
			.add_method("set_user_number", &T::set_user_number, "", "NumberProvider")
			.add_method("print", &T::print, "Result", "x#y");

	typedef PrintUserVector2d T2;
	std::stringstream ss2;
	ss2 << "PrintUserVector2d";
	reg.add_class_<T2 > (ss2.str().c_str(), parentGroup)
			.add_constructor()
			.add_method("set_user_vector", &T2::set_user_vector, "", "NumberProvider")
			.add_method("print", &T2::print, "Result", "x#y");

	typedef PrintBoundaryNumber2d T3;
	std::stringstream ss3;
	ss3 << "PrintBoundaryNumber2d";
	reg.add_class_<T3 > (ss3.str().c_str(), parentGroup)
			.add_constructor()
			.add_method("set_user_number", &T3::set_user_number, "", "BoundaryNumber")
			.add_method("print", &T3::print, "Result", "x#y");


#endif
#ifdef UG_DIM_3
	paramNames.clear();
	paramNames.push_back("x");
	paramNames.push_back("y");
	paramNames.push_back("z");
	paramNames.push_back("t");
	ug::vrl::RegisterUserData < 3 > (reg, paramNames, parentGroup);
#endif
}

} // vrl::
} // ug::
