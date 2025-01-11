#ifndef TYPES_HPP
#define TYPES_HPP

#include "utils.hpp"

namespace Constants
{
    const int MIN_INT = -2147483648;
    const int MAX_INT = 2147483647;
    const int MIN_CHAR = 0;
    const int MAX_CHAR = 255;

    inline bool isValidNumConstant(int value)
    {
        return value >= MIN_INT && value <= MAX_INT;
    }

    inline bool isValidNumConstant(char value)
    {
        return value >= MIN_CHAR && value <= MAX_CHAR;
    }

    inline bool isValidEscapeSequence(char c)
    {
        return c == 't' || c == 'n' || c == '0' ||
               c == '\'' || c == '\"' || c == '\\';
    }
}

enum class BasicType
{
    INT,
    CHAR,
    VOID,
};

class TypeInfo;

namespace TypeUtils
{
    inline static bool isNumericType(const TypeInfo &type);
    inline static bool isFunctionType(const TypeInfo &type);
    inline static bool isArrayType(const TypeInfo &type);
    static TypeInfo makeArrayType(BasicType type, bool constQualified = false);
    static TypeInfo makeFunctionType(BasicType returnType = BasicType::VOID, vector<TypeInfo> params = vector<TypeInfo>());
    static bool areTypesCompatible(const TypeInfo &source, const TypeInfo &target);
    static string typeToString(const BasicType &type);
    static string typeToString(const TypeInfo &type);
    static std::string concatToString(const vector<TypeInfo> &c, const std::string delim = " ");
}

struct TypeInfo
{
private:
    BasicType baseType = BasicType::VOID;
    bool isConst_ = false;
    bool isArray_ = false;

    // For function types
    vector<TypeInfo> functionParams;
    BasicType &returnType = baseType;
    
    //const static TypeInfo VOID;  // Only declare it

public:
    TypeInfo() {}

    static const TypeInfo VOID; // Universal VOID TypeInfo
    //inline static const TypeInfo VOID = TypeInfo(BasicType::VOID);  

    
    // Constructor for variable types
    TypeInfo(BasicType type, bool constQualified = false, bool array = false)
        : baseType(type), isConst_(constQualified), isArray_(array) {}

    // Constructor for function types
    TypeInfo(BasicType returnType, vector<TypeInfo> params)
        : baseType(returnType), functionParams(!params.empty() ? params : vector<TypeInfo>{TypeInfo::VOID}) {}

    
    //inline static const TypeInfo VOID{BasicType::VOID};

    // Method to check implicit type conversion
    bool canImplicitlyConvertTo(const TypeInfo &target) const
    {
        return TypeUtils::areTypesCompatible(*this, target);
    }
    bool canImplicitlyConvertTo(const BasicType &target) const
    {
        return canImplicitlyConvertTo(TypeInfo(target));
    }

    // Getters
    BasicType getBaseType() const { return baseType; }
    BasicType getReturnType() const { return returnType; }
    bool isConst() const { return isConst_; }
    bool isArray() const { return isArray_; }
    bool isFunc() const { return !functionParams.empty(); }
    const vector<TypeInfo> &getFunctionParams() const { return functionParams; }
    bool isVoidParam() const { return functionParams[0] == TypeInfo::VOID && functionParams.size() == 1; }
    bool isVoid() const { return baseType == BasicType::VOID && !isFunc(); }
    bool lValue() const { return !isConst() && TypeUtils::isNumericType(*this); }

    // Conversion to string
    string toString() const
    {
        return TypeUtils::typeToString(*this);
    }
    string operator()() const
    {
        return toString();
    }

    // Operators
    bool operator==(const TypeInfo &other) const
    {
        return baseType == other.baseType &&
               isConst() == other.isConst() &&
               isArray() == other.isArray() &&
               isFunc() == other.isFunc() &&
               functionParams == other.functionParams &&
               returnType == other.returnType;
    }

    bool operator!=(const TypeInfo &other) const
    {
        return !(*this == other);
    }

    TypeInfo &operator=(const TypeInfo &other)
    {
        this->baseType = other.getBaseType();
        this->isConst_ = other.isConst();
        this->isArray_ = other.isArray();
        this->functionParams = other.getFunctionParams(); // For function types
        return *this;
    }
};

//const TypeInfo TypeInfo::VOID = TypeInfo(BasicType::VOID); // TEO commented this
//const static TypeInfo VOID;  // Only declare it
//const TypeInfo TypeInfo::VOID(BasicType::VOID);




namespace TypeUtils
{
    inline static string typeToString(const BasicType &type)
    {
        switch (type)
        {
        case BasicType::INT:
            return "int";
        case BasicType::CHAR:
            return "char";
        case BasicType::VOID:
            return "void";
        default:
            return "unknown";
        }
    }
    inline static string typeToString(const TypeInfo &type)
    {
        string result = TypeUtils::typeToString(type.getBaseType());
        if (type.isConst())
            result = "const(" + result + ")";
        if (type.isArray())
            result = "niz(" + result + ")";
        if (type.isFunc())
        {
            result = "funkcija(";

            if (type.isVoidParam())
                result += "void";
            else
                result += "[" + concatToString(type.getFunctionParams(), ",") + "]";

            result += " -> " + TypeUtils::typeToString(type.getReturnType()) + ")";
        }
        return result;
    }

    inline static bool isNumericType(const TypeInfo &type)
    {
        return !type.isFunc() && !type.isArray() && type.getBaseType() == BasicType::INT ||
               type.getBaseType() == BasicType::CHAR;
    }

    inline bool isFunctionType(const TypeInfo &type)
    {
        return type.isFunc();
    }

    inline bool isArrayType(const TypeInfo &type)
    {
        return type.isArray();
    }

    inline static bool areTypesCompatible(const TypeInfo &source, const TypeInfo &target)
    {
        // Same types always compatible
        if (source == target)
            return true;

        // Handle void, Array discrepancy and Functions
        if (source.isVoid() || target.isVoid() ||
            source.isArray() != target.isArray() ||
            source.isFunc() || target.isFunc())
            return false;

        // Handle const relationships for arrays and vars
        if (source.getBaseType() == target.getBaseType())
            return !source.isConst();

        // Handle char -> int conversion
        if (source.getBaseType() == BasicType::CHAR &&
            target.getBaseType() == BasicType::INT &&
            !source.isArray() && !source.isConst() && !target.isConst())
            return true;

        // Everything else is incompatible
        return false;
    }

    // Static factory methods
    static TypeInfo makeArrayType(BasicType type, bool constQualified)
    {
        return TypeInfo(type, constQualified, true);
    }

    static TypeInfo makeFunctionType(BasicType returnType, vector<TypeInfo> params)
    {
        return TypeInfo(returnType, params);
    }

    static std::string concatToString(const vector<TypeInfo> &c, const std::string delim)
    {
        std::string rez = "";
        for (int i = (int)c.size() - 1; i > -1; i--)
        {
            rez += c[i].toString();
            if (i)
                rez += delim;
        }
        return rez;
    }

    
}



const TypeInfo TypeInfo::VOID(BasicType::VOID);

#endif

