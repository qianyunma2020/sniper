#ifndef SNIPER_ALP_ReturnCode_H
#define SNIPER_ALP_ReturnCode_H

#include <ostream>
#include <type_traits>
#include <utility>

template <typename T>
struct is_ALPReturnCode_enum : std::false_type {}; 

class ALPReturnCode final
{
public:
	typedef long int ReturnCodeType; //返回码类型
	enum class ReturnCode : ReturnCodeType { FAILURE = 0, SUCCESS = 1, RECOVERABLE = 2 };  //枚举类
	//以下常量表达式用于快速访问状态码
	constexpr const static auto FAILURE = ReturnCode::FAILURE;
	constexpr const static auto SUCCESS = ReturnCode::SUCCESS;
	constexpr const static auto RECOVERABLE = ReturnCode::RECOVERABLE;


	//值的判断
	bool IsSuccess(){
		return static_cast<ReturnCodeType>(ReturnCode::SUCCESS) == m_ReturnCode;
	}
	bool IsSuccess(ReturnCodeType tmpCode) {
		return static_cast<ReturnCodeType>(ReturnCode::SUCCESS) == tmpCode; 
	}
	bool IsFailure(){
		return static_cast<ReturnCodeType>(ReturnCode::FAILURE) == m_ReturnCode;
	}
	bool IsFailure(ReturnCodeType tmpCode){
		return static_cast<ReturnCodeType>(ReturnCode::FAILURE) == tmpCode;
	}
	bool IsRecoverable() {
		return static_cast<ReturnCodeType>(ReturnCode::RECOVERABLE) == m_ReturnCode;
	}
	bool IsRecoverable(ReturnCodeType tmpCode){
		return static_cast<ReturnCodeType>(ReturnCode::RECOVERABLE) == tmpCode;
	}
	//取值
	ReturnCodeType GetValue() {
		switch (m_ReturnCode)
		{
		case static_cast<ReturnCodeType>(ReturnCode::SUCCESS):      return m_ReturnCode;
		case static_cast<ReturnCodeType>(ReturnCode::FAILURE):      return m_ReturnCode;
		case static_cast<ReturnCodeType>(ReturnCode::RECOVERABLE):  return m_ReturnCode;
		default:
			m_ReturnCode = static_cast<ReturnCodeType>(ReturnCode::FAILURE);
			return m_ReturnCode;
		}
	}

public:
       template <typename T, typename = std::enable_if<is_ALPReturnCode_enum<T>::value >> 
       ALPReturnCode(T value) noexcept {
            *this = ALPReturnCode( static_cast<ALPReturnCode::ReturnCodeType>(value)); 
       }
       
       explicit ALPReturnCode(ReturnCodeType tmpCode) noexcept : m_ReturnCode(tmpCode) { }

       ALPReturnCode& operator=( const ALPReturnCode& tmpCode) noexcept{
           m_ReturnCode = tmpCode.m_ReturnCode; 
           return *this;
       }


	//构造函数
	ALPReturnCode() {
		m_ReturnCode = static_cast<ReturnCodeType>(ReturnCode::SUCCESS);
	}
	ALPReturnCode(ALPReturnCode& tmpALPReturnCode) noexcept{
		m_ReturnCode = tmpALPReturnCode.m_ReturnCode;
	}
	ALPReturnCode(ALPReturnCode&& tmpALPReturnCode) noexcept {
		m_ReturnCode = tmpALPReturnCode.m_ReturnCode;
	}
	~ALPReturnCode() {}

	//比较操作
	friend bool operator==(const ALPReturnCode& ltmp, const ALPReturnCode& rtmp) {
		if ((ltmp.m_ReturnCode == rtmp.m_ReturnCode) && ((static_cast<ReturnCodeType>(ReturnCode::SUCCESS) == ltmp.m_ReturnCode) || (static_cast<ReturnCodeType>(ReturnCode::FAILURE) == ltmp.m_ReturnCode)))
		{
			return true;
		}
		return false;
	}
	friend bool operator!=(const ALPReturnCode& ltmp, const ALPReturnCode& rtmp)
	{
		return !(ltmp == rtmp);
	}
	//逻辑与或操作
	ALPReturnCode& operator&=(ALPReturnCode& rtmp)
	{
		static constexpr ALPReturnCode::ReturnCodeType AND[3][3] = { {0, 0, 0},{0, 1, 2},{0, 2, 2} };
		ALPReturnCode::ReturnCodeType first = static_cast<ReturnCodeType>(GetValue());
		ALPReturnCode::ReturnCodeType second = static_cast<ReturnCodeType>(rtmp.GetValue());
		m_ReturnCode = AND[first][second];
		return *this;
	}

	ALPReturnCode& operator|=(ALPReturnCode& rtmp)
	{
		static constexpr ALPReturnCode::ReturnCodeType OR[3][3] = { {0, 1, 2}, {1, 1, 1}, {2, 1, 2} };
		ALPReturnCode::ReturnCodeType first = static_cast<ReturnCodeType>(GetValue());
		ALPReturnCode::ReturnCodeType second = static_cast<ReturnCodeType>(rtmp.GetValue());
		m_ReturnCode = OR[first][second];
		return *this;
	}

public:
	friend std::ostream& operator<<(std::ostream& os, ALPReturnCode& tmpALPReturnCode)
	{
		os << "Value is: "<<std::to_string(tmpALPReturnCode.GetValue())<<"\n";
		return os;
	}

private:
	ReturnCodeType m_ReturnCode;

};

//两个对象的与或操作
inline ALPReturnCode& operator&(ALPReturnCode ltmp, ALPReturnCode& rtmp) { 
	return ltmp &= rtmp; 
}
inline ALPReturnCode& operator|(ALPReturnCode ltmp, ALPReturnCode& rtmp) {
	return ltmp |= rtmp;
}
//布尔值与对象的与或操作
inline bool& operator&=(bool& ltmp, ALPReturnCode& rtmp) {
	return ltmp &= rtmp.IsSuccess();
}
inline bool& operator|=(bool& ltmp, ALPReturnCode& rtmp) {
	return ltmp |= rtmp.IsSuccess();
}
#endif
