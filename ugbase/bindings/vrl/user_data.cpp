#include "bridge/bridge.h"
#include "type_converter.h"
#include "common/common.h"
#include "lib_disc/spatial_disc/ip_data/const_user_data.h"
#include "bindings/lua/lua_util.h"
#include "bindings_vrl.h"
#include "type_converter.h"
#include "threading.h"
#include <iostream>
#include <sstream>
#include <boost/function.hpp>
#include <jni.h>
#include "lib_disc/spatial_disc/ip_data/ip_data.h"

namespace ug {
namespace vrl {

template <std::size_t dim>
inline jdoubleArray ConvertParametersToJava(JNIEnv *env, const MathVector<dim>& x,
                                            const number time)
{
	jdoubleArray array = NULL;
	array = env->NewDoubleArray(dim+1);

	jdouble vTmp[dim + 1];
	for (size_t i = 0; i < dim; i++) vTmp[i] = x[i];
	vTmp[dim] = time;

	env->SetDoubleArrayRegion(array, 0, dim+1, vTmp);
	checkException(env, "CopyParametersToJava: error");

	return array;
}

template <typename TData>
struct vrl_traits;

template <>
struct vrl_traits<number>
{
	typedef jdouble jType;
	static const unsigned int retArrayDim = 0;
	static const int size = 1;
	static std::string name() {return "Number";}
	static std::string errMsg()
	{
		std::stringstream ss;
		ss << "<font color=\"red\">VRLUserNumber: invokation error:</font>";
		return ss.str();
	}
	static std::string callSignature() {return "([DI)D";}

	static void toJava(JNIEnv *env, jdouble& res, const number& x)
	{
		res = x;
	}

	static void toC(JNIEnv *env, number& res, jdouble& from)
	{
		res = from;
	}

	static void call(JNIEnv *env, number& res,
	                 jobject obj, jmethodID method,
	                 jdoubleArray params, jint si)
	{
		res = env->CallDoubleMethod(obj, method, params, si);
		if (checkException(env, errMsg())) {}
	}
};

template <std::size_t dim>
struct vrl_traits<ug::MathVector<dim> >
{
	typedef jdoubleArray jType;
	static const unsigned int retArrayDim = 1;
	static const int size = dim;
	static std::string name() {return "Vector";}
	static std::string errMsg()
	{
		std::stringstream ss;
		ss << "<font color=\"red\">VRLUserVector"<<dim<<"d: invokation error:</font>";
		return ss.str();
	}
	static std::string callSignature() {return "([DI)[D";}

	static void toJava(JNIEnv *env, jdoubleArray& res, const MathVector<dim>& x)
	{
		jdouble vTmp[size];
		for (jint i = 0; i < dim; i++) vTmp[i] = x[i];
		env->SetDoubleArrayRegion(res, 0, size, vTmp);
	}

	static void toC(JNIEnv *env, MathVector<dim>& res, jdoubleArray& array)
	{
		jint arrayLength = env->GetArrayLength(array);
		if (arrayLength != size)
			UG_THROW(RED_BEGIN << "VRLUserVector: dimensions do not match! "
			               "Required: "<<size<<", returned: " << arrayLength <<
			               COLOR_END << std::endl);

		jdouble vTmp[size];
		env->GetDoubleArrayRegion(array, 0, size, vTmp);
		for (jint i = 0; i < (jint)dim; ++i) res[i] = vTmp[i];
	}

	static void call(JNIEnv *env, MathVector<dim>& res,
	                 jobject obj, jmethodID method,
	                 jdoubleArray params, jint si)
	{
		jdoubleArray tmp =
				(jdoubleArray) env->CallObjectMethod(obj, method, params, si);

		if(checkException(env, errMsg()))
		{
			toC(env, res, tmp);
		}
	}
};

template <std::size_t dim>
struct vrl_traits<ug::MathMatrix<dim,dim> >
{
	typedef jobjectArray jType;
	static const unsigned int retArrayDim = 2;
	static const int size = dim*dim;
	static std::string name() {return "Matrix";}
	static std::string errMsg()
	{
		std::stringstream ss;
		ss << "<font color=\"red\">VRLUserMatrix"<<dim<<"d: invokation error:</font>";
		return ss.str();
	}
	static std::string callSignature() {return "([DI)[[D";}

	static void toJava(JNIEnv *env, jobjectArray& res, const MathMatrix<dim,dim>& x)
	{
		UG_THROW("Not implemented.");
	}

	static void toC(JNIEnv *env, MathMatrix<dim,dim>& mat, jobjectArray& array)
	{
		const int rowSize = env->GetArrayLength(array);
		if(rowSize != dim)
			UG_THROW(RED_BEGIN << "VRLUserMatrix: wrong row size! Required:"
			               <<dim<<", returned: "<<rowSize<<COLOR_END << std::endl);

		jdouble rowEntrys[dim];

		for(int i=0; i < (int)dim; ++i){
			jdoubleArray row = (jdoubleArray)env->GetObjectArrayElement(array, i);

			const int colSize = env->GetArrayLength(row);
			if(colSize != dim)
				UG_THROW(RED_BEGIN << "VRLUserMatrix: wrong column size! Required:"
							   <<dim<<", returned: "<<colSize<<COLOR_END << std::endl);

			env->GetDoubleArrayRegion(row, 0, dim, rowEntrys);
			for(int j=0; j < (int)dim; ++j)
				mat[i][j]= rowEntrys[j];

			// alternative is to force garbageCollector to "pin" or copy internally
//			jdouble* rowEntrys = env->GetDoubleArrayElements(row, 0);
//			env->ReleaseDoubleArrayElements(row, rowEntrys, 0);
		}
	}

	static void call(JNIEnv *env, MathMatrix<dim,dim>& res,
	                 jobject obj, jmethodID method,
	                 jdoubleArray params, jint si)
	{
		jobjectArray tmp =
				(jobjectArray) env->CallObjectMethod(obj, method, params, si);

		if(checkException(env, errMsg()))
		{
			toC(env, res, tmp);
		}
	}

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


template <typename TData, int dim>
class VRLUserData : public IPData<TData, dim>
{
	protected:
		static jobject compileUserDataString(JNIEnv *env, const char* s)
		{
			jclass cls = env->FindClass("edu/gcsc/vrl/ug/UserDataCompiler");
			if (env->ExceptionCheck()) {env->ExceptionDescribe();}

			jmethodID runMethod = env->GetStaticMethodID(
					cls, "compile",
					"(Ljava/lang/String;I)Ljava/lang/Object;");
			if (env->ExceptionCheck()) {env->ExceptionDescribe();}

			return env->CallStaticObjectMethod(cls, runMethod, stringC2J(env, s),
			                                   retArrayDim);
		}

		static jclass getUserDataClass(JNIEnv *env)
		{
			jclass result = env->FindClass("edu/gcsc/vrl/ug/UserData");
			if (env->ExceptionCheck()) {env->ExceptionDescribe();}

			return result;
		}

		static jmethodID getUserDataRunMethod(JNIEnv *env, jclass cls)
		{
			std::string signature = vrl_traits<TData>::callSignature();
			std::stringstream mName; mName << "run" << retArrayDim;
			jmethodID result = env->GetMethodID(cls, mName.str().c_str(), signature.c_str());

			if (!checkException(env))
			{
				UG_LOG("[VRL-Bindings] "<<name()<<" Error:"
						<< " cannot find userdata method."
						<< " Please check your implementation!" << std::endl);
			}
			return result;
		}

	public:
		static const unsigned int retArrayDim = vrl_traits<TData>::retArrayDim;

		VRLUserData() : initialized(false) {}

		static std::string params()
		{
			// DO NOT USE underscore for param names (i.e. NO "_x" !!!)
			std::stringstream params;
			if(dim >= 1) params <<  "\"" << "x" << "\"";
			if(dim >= 2) params << ",\"" << "y" << "\"";
			if(dim >= 3) params << ",\"" << "z" << "\"";
						 params << ",\"" << "t" << "\"";
						 params << ",\"" << "si" << "\"";
			return params.str();
		}

		static std::string name()
		{
			std::stringstream ss;
			ss << "VRLUser"<<vrl_traits<TData>::name() << dim << "d";
			return ss.str();
		}

		void set_vrl_callback(const char* expression)
		{
			JNIEnv* env = threading::getEnv(getJavaVM());

			releaseGlobalRefs();

			userDataObject = compileUserDataString(env, expression);
			userDataClass = getUserDataClass(env);
			runMethod = getUserDataRunMethod(env, userDataClass);

			checkException(env, name().append(": Cannot setup evaluation class or method."));

			// create thread-safe references
			// (GC won't deallocate them until manual deletion request)
			userDataObject = env->NewGlobalRef(userDataObject);
			userDataClass = (jclass) env->NewGlobalRef((jobject) userDataClass);
			checkException(env, name().append(": Global Reference Error."));
			initialized = true;
		}

		///	evaluates the data at a given point and time
		void operator() (TData& c, const MathVector<dim>& x, number time, int si) const
		{
			JNIEnv* env = threading::getEnv(getJavaVM());

			// convert parameters
			jdoubleArray params = ConvertParametersToJava(env, x, time);
			jint jsi = si;

			if (runMethod != NULL)
				vrl_traits<TData>::call(env, c, userDataObject, runMethod,
				                        params, jsi);
		}

		void releaseGlobalRefs()
		{
			// deleting thread-safe global references
			if (initialized) {
				JNIEnv* localEnv = threading::getEnv(getJavaVM());
				localEnv->DeleteGlobalRef(userDataObject);
				localEnv->DeleteGlobalRef((jobject) userDataClass);
			}
		}

		~VRLUserData()
		{
			releaseGlobalRefs();
		}

	protected:
		/// Base class type
		typedef IPData<TData, dim> base_type;

		using base_type::num_series;
		using base_type::num_ip;
		using base_type::ip;
		using base_type::time;
		using base_type::subset;
		using base_type::value;

		///	implement as a IPData
		virtual void compute(bool computeDeriv = false)
		{
			for (size_t s = 0; s < num_series(); ++s)
			{
				for (size_t i = 0; i < num_ip(s); ++i)
				{
					this->operator()(value(s, i), ip(s, i), time(), subset());
				}
			}
		}

	private:
		bool initialized;
		jobject userDataObject;
		jclass userDataClass;
		jmethodID runMethod;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <int dim>
class VRLCondUserNumber : public IPData<number, dim, bool>
{
protected:
	jdouble condData2Double(JNIEnv *env, jobject obj) const
	{
		// todo: cache this
		jclass cls = env->FindClass("edu/gcsc/vrl/ug/Cond");
		if (env->ExceptionCheck()) {env->ExceptionDescribe();}

		jmethodID method = env->GetMethodID(cls, "getValue", "()D");
		if (env->ExceptionCheck()) {env->ExceptionDescribe();}

		return env->CallDoubleMethod(obj, method);
	}

	jdouble condData2Boolean(JNIEnv *env, jobject obj) const
	{
		// todo: cache this
		jclass cls = env->FindClass("edu/gcsc/vrl/ug/Cond");
		if (env->ExceptionCheck()) {env->ExceptionDescribe();}

		jmethodID method = env->GetMethodID(cls, "getCondBool", "()Z");
		if (env->ExceptionCheck()) {env->ExceptionDescribe();}

		return env->CallBooleanMethod(obj, method);
	}

	jobject compileCondUserDataString(JNIEnv *env, const char* s) const
	{
		jclass cls = env->FindClass("edu/gcsc/vrl/ug/CondUserDataCompiler");
		if (env->ExceptionCheck()) {env->ExceptionDescribe();}

		jmethodID runMethod = env->GetStaticMethodID(
				cls, "compile",
				"(Ljava/lang/String;)Ljava/lang/Object;");
		if (env->ExceptionCheck()) {env->ExceptionDescribe();}

		return env->CallStaticObjectMethod(cls, runMethod, stringC2J(env, s));
	}

	jclass getCondUserDataClass(JNIEnv *env) const
	{
		jclass result = env->FindClass("edu/gcsc/vrl/ug/CondUserData");
		if (env->ExceptionCheck()) {env->ExceptionDescribe();}

		return result;
	}

	jmethodID getCondUserDataRunMethod(JNIEnv *env, jclass cls) const
	{
		std::string signature = "([DI)Ledu/gcsc/vrl/ug/Cond;";
		std::string mName = "run";

		jmethodID result = env->GetMethodID(cls, mName.c_str(), signature.c_str());
		if (!checkException(env))
		{
			UG_LOG("[VRL-Bindings] Error:"
					<< " cannot find cond-userdata method."
					<< " Please check your implementation!" << std::endl);
		}

		return result;
	}

public:
	static std::string params()
	{
		// DO NOT USE underscore for param names (i.e. NO "_x" !!!)
		std::stringstream params;
		if(dim >= 1) params <<  "\"" << "x" << "\"";
		if(dim >= 2) params << ",\"" << "y" << "\"";
		if(dim >= 3) params << ",\"" << "z" << "\"";
					 params << ",\"" << "t" << "\"";
					 params << ",\"" << "si" << "\"";
		return params.str();
	}

	static std::string name()
	{
		std::stringstream ss;
		ss << "VRLCondUser"<<vrl_traits<number>::name() << dim << "d";
		return ss.str();
	}

	VRLCondUserNumber() : initialized(false)
	{
		std::stringstream stream;
		stream << "<font color=\"red\">VRLCondUserNumber"
				<< dim << "D: invokation error:</font>";
		invocationErrorMsg = stream.str();
	}

	void set_vrl_callback(const char* expression)
	{
		JNIEnv* env = threading::getEnv(getJavaVM());

		releaseGlobalRefs();

		userDataObject = compileCondUserDataString(env, expression);
		userDataClass = getCondUserDataClass(env);
		runMethod = getCondUserDataRunMethod(env, userDataClass);

		// create thread-safe references 
		// (GC won't deallocate them until manual deletion request)
		userDataObject = env->NewGlobalRef(userDataObject);
		userDataClass = (jclass) env->NewGlobalRef((jobject) userDataClass);

		initialized = true;
	}

	///	evaluates the data at a given point and time
	bool operator() (number& c, const MathVector<dim>& x, number time, int si) const
	{
		JNIEnv* env = threading::getEnv(getJavaVM());

		// convert parameters
		jdoubleArray params = ConvertParametersToJava(env, x, time);
		jint jsi = si;

		bool result = false;
		if (runMethod != NULL)
		{
			jobject bndResult = (jdoubleArray) env->CallObjectMethod(
					userDataObject,
					runMethod,
					params, jsi);

			if (checkException(env, invocationErrorMsg)) {
				result = condData2Boolean(env, bndResult);
				c = condData2Double(env, bndResult);
			}
		}

		return result;
	}

/// Base class type
	typedef IPData<number, dim, bool> base_type;

	using base_type::num_series;
	using base_type::num_ip;
	using base_type::ip;
	using base_type::time;
	using base_type::subset;
	using base_type::value;


	virtual void compute(bool computeDeriv = false)
	{
		// \todo: should remember flag
		for (size_t s = 0; s < num_series(); ++s)
		{
			for (size_t i = 0; i < num_ip(s); ++i)
			{
				this->operator()(value(s, i), ip(s, i), time(), subset());
			}
		}
	}

	void releaseGlobalRefs()
	{
		// deleting thread-safe global references
		if (initialized) {
			JNIEnv* localEnv = threading::getEnv(getJavaVM());
			localEnv->DeleteGlobalRef(userDataObject);
			localEnv->DeleteGlobalRef((jobject) userDataClass);
		}
	}

	~VRLCondUserNumber() {
		releaseGlobalRefs();
	}

private:
	std::string invocationErrorMsg;
	bool initialized;
	jobject userDataObject;
	jclass userDataClass;
	jmethodID runMethod;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <typename TData>
class PrintUserData2d {
public:
	void set(SmartPtr<IPData<TData, 2> > user) {m_spNumber = user;}

	static std::string dataname()
	{
		std::stringstream ss; ss << "User"<<vrl_traits<TData>::name()<<"2d";
		return ss.str();
	}

	static std::string name()
	{
		std::stringstream ss; ss << "PrintUser"<<vrl_traits<TData>::name()<<"2d";
		return ss.str();
	}

	std::string print(number x, number y, number time, int si)
	{
		MathVector < 2 > v(x, y);
		TData ret;

		if (m_spNumber.valid()) (*m_spNumber)(ret, v, time, si);
		else UG_THROW(name()<<": Data not set.");

		std::stringstream ss;
		ss << ret << std::endl;
		return ss.str();
	}

private:
	SmartPtr<IPData<TData, 2> > m_spNumber;
};

template <typename TData>
class PrintCondUserData2d {
public:
	static std::string dataname()
	{
		std::stringstream ss; ss << "CondUser"<<vrl_traits<TData>::name()<<"2d";
		return ss.str();
	}

	static std::string name()
	{
		std::stringstream ss; ss << "PrintCondUser"<<vrl_traits<TData>::name()<<"2d";
		return ss.str();
	}

	void set(SmartPtr<IPData<number, 2, bool> > user) {m_spData = user;}

	std::string print(number x, number y, number time, int si)
	{
		MathVector < 2 > v(x, y);
		number ret;
		bool bndResult = false;

		if (m_spData.valid()) bndResult = (*m_spData)(ret, v, time, si);
		else {
			UG_THROW(name()<<": Data not set.");
			ret = -1;
		}

		std::stringstream stream;
		stream << "[";
		if(bndResult) stream << "true";
		else stream << "false";
		stream << ", " << ret << "]";
		return stream.str();
	}

private:
	SmartPtr<IPData<number, 2, bool> > m_spData;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template <typename TData, int dim>
void RegisterUserDataType(ug::bridge::Registry& reg, const std::string& grp)
{
	//	VRLUserType
	{
		typedef VRLUserData<TData, dim> T;
		typedef IPData<TData, dim> TBase;
		std::stringstream options;
		options << "Input:|user-data|dim=" << T::retArrayDim << ";"
				<< "params=["<<T::params()<<"];";
		reg.add_class_<T, TBase>(T::name(), grp)
			.add_constructor()
			.add_method("data", &T::set_vrl_callback, "", options.str().c_str())
			.set_construct_as_smart_pointer(true);
	}

	// PrintUserType2d
	if(dim == 2)
	{
		typedef PrintUserData2d<TData> T;
		reg.add_class_<T > (T::name(), grp)
				.add_constructor()
				.add_method("set", &T::set, "", T::dataname())
				.add_method("print", &T::print, "Result", "x#y#t#si");
	}
}

template <int dim>
void RegisterUserData(ug::bridge::Registry& reg, const char* parentGroup)
{
	// 	get group
	std::string grp = std::string(parentGroup);

	RegisterUserDataType<number, dim>(reg, grp);
	RegisterUserDataType<MathVector<dim>, dim>(reg, grp);
	RegisterUserDataType<MathMatrix<dim,dim>, dim>(reg, grp);

	//	VRLCondUserNumber
	{
		typedef VRLCondUserNumber<dim> T;
		typedef IPData<number, dim, bool> TBase;
		std::stringstream options;
		options << "Input:|cond-user-data|params=["<<T::params()<<"];";
		reg.add_class_<T, TBase>(T::name(), grp)
			.add_constructor()
			.add_method("data", &T::set_vrl_callback, "", options.str().c_str())
			.set_construct_as_smart_pointer(true);
	}

	if(dim == 2)
	{
		typedef PrintCondUserData2d<number> T3;
		reg.add_class_<T3 > (T3::name(), grp)
				.add_constructor()
				.add_method("set", &T3::set, "", T3::dataname())
				.add_method("print", &T3::print, "Result", "x#y#t#si");
	}
}

void RegisterUserData(ug::bridge::Registry& reg, const char* parentGroup)
{
#ifdef UG_DIM_1
	ug::vrl::RegisterBridge_UserData < 1 > (reg, parentGroup);
#endif
#ifdef UG_DIM_2
	ug::vrl::RegisterBridge_UserData < 2 > (reg, parentGroup);
#endif
#ifdef UG_DIM_3
	ug::vrl::RegisterBridge_UserData < 3 > (reg, parentGroup);
#endif
}

} // vrl::
} // ug::
