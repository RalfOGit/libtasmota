#ifndef __LIBTASMOTA_JSONCPPWRAPPER_HPP__
#define __LIBTASMOTA_JSONCPPWRAPPER_HPP__

#include <Json.hpp>
#include <cstdint>
#include <string>
#include <vector>
#include <variant>
#include <utility>


namespace libtasmota {

    /**
     * Class implementing a C++ wrapper for the very low-print json parsing library written by James McLaughlin
     * (see https://github.com/json-parser/json-parser).
     * It implements 
     */
    class JsonCppWrapper {
    private:
        //JsonCppWrapper(void) {}
        //~JsonCppWrapper(void) {}

    public:

        /** Class encapsulating a json object value. */
        class JsonObject {
        protected:
            json_object_entry* value;   ///< pointer to an array of json_object_entry elements
            unsigned int       length;  ///< number of json_object_entry elements in the array
        public:
            JsonObject(const json_object_entry* const entry = NULL) : value(NULL), length(0) {              /// Constructor. @param pointer to the json_object_entry in the json tree
                if (entry != NULL &&  entry->value != NULL && entry->value->type == json_object) {
                    value  = entry->value->u.object.values;
                    length = entry->value->u.object.length;
                }
            }
            const json_object_entry* const getElements     (void) const { return value; }                   ///< Get child elements for this json object. 
            const unsigned int             getNumElements  (void) const { return length; }                  ///< Get number of child elements for this json object.
            std::string                    getValueAsString(void) const { return "SORRY NOT IMPLEMENTED"; } ///< Get a string representation for this json object => not yet implemented.
            const json_object_entry& operator[](size_t index) { return value[index]; }                      ///< Array access operator [] for this json object.
        };

        /** Class encapsulating a json array value. */
        class JsonArray {
        protected:
            json_value** value;     ///< pointer to an array of json_value elements
            unsigned int length;    ///< number of json_value elements in the array
        public:
            JsonArray(const json_object_entry* const entry = NULL) : value(NULL), length(0) {                   /// Constructor. @param pointer to the json_object_entry in the json tree
                if (entry != NULL && entry->value != NULL && entry->value->type == json_array) {
                    value  = entry->value->u.array.values;
                    length = entry->value->u.array.length;
                }
            }
            const json_value** const getElements     (void) const { return (const json_value** const)value; }   ///< Get child elements for this json array. 
            const int64_t            getNumElements  (void) const { return length; }                            ///< Get number of child elements for this json array.
            std::string              getValueAsString(void) const { return "SORRY NOT IMPLEMENTED"; }           ///< Get a string representation for this json array => not yet implemented.
            const json_value& operator[](size_t index) { return (*value)[index]; }                              ///< Array access operator [] for this json array.
        };

        /** Class encapsulating a json string value. */
        class JsonString {
        protected:
            std::string value;      ///< json string value converted from the json_object_entry
        public:
            JsonString(const json_object_entry* const entry = NULL) : value("INVALID") {            /// Constructor. @param pointer to the json_object_entry in the json tree
                if (entry != NULL &&entry->value != NULL && entry->value->type == json_string) {
                    value = std::string(entry->value->u.string.ptr, entry->value->u.string.length);
                }
            }
            const std::string& getValue(void)         const { return value; }                       ///< Get string value for this json string.
            const std::string& getValueAsString(void) const { return value; }                       ///< Get string value for this json string. Same as getValue().
        };

        /** Class encapsulating a json integer value. */
        class JsonInt {
        protected:
            long long value;                                                                        ///< json integer value copied from the json_object_entry
        public:
            JsonInt(const json_object_entry* const entry = NULL) : value(-99999999) {               /// Constructor. @param pointer to the json_object_entry in the json tree
                if (entry != NULL && entry->value != NULL && entry->value->type == json_integer) {
                    value = entry->value->u.integer;
                }
            }
            long long   getValue        (void) const { return value; }                              ///< Get value of this json integer.
            std::string getValueAsString(void) const {                                              ///< Get string representation for this json integer.
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%lld", value);
                return buffer;
            }
        };

        /** Class encapsulating a json double value. */
        class JsonDouble {
        protected:
            double value;                                                                           ///< json double value copied from the json_object_entry
        public:
            JsonDouble(const json_object_entry* const entry = NULL) : value(-99999999) {            /// Constructor. @param pointer to the json_object_entry in the json tree
                if (entry != NULL && entry->value != NULL && entry->value->type == json_double) {
                    value = entry->value->u.dbl;
                }
            }
            double      getValue        (void) const { return value; }                              ///< Get value of this json double.
            std::string getValueAsString(void) const {                                              ///< Get string representation for this json double.
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%lf", value);
                return buffer;
            }
        };

        /** Class encapsulating a json boolean value. */
        class JsonBool {
        protected:
            bool value;
        public:
            JsonBool(const json_object_entry* const entry = NULL) : value(false) {                  /// Constructor. @param pointer to the json_object_entry in the json tree
                if (entry != NULL && entry->value != NULL && entry->value->type == json_boolean) {
                    value = entry->value->u.boolean;
                }
            }
            bool        getValue        (void) const { return value; }                              ///< Get value of this json boolean.
            std::string getValueAsString(void) const {                                              ///< Get string representation for this json boolean.
                return (value == false ? std::string("false") : std::string("true"));
            }
        };

        /** Class encapsulating a json name value pair. It supports all possible json value types. */
        class JsonNamedValue {
        protected:
            std::string name;
            JsonObject  value_object;
            JsonArray   value_array;
            JsonString  value_string;
            JsonBool    value_boolean;
            JsonInt     value_int;
            JsonDouble  value_double;
            json_type   type;
        public:
            JsonNamedValue(const json_object_entry* const entry = NULL) {
                if (entry != NULL && entry->name != NULL && entry->value != NULL) {
                    name = std::string(entry->name, entry->name_length);
                    switch (entry->value->type) {
                    case json_object:   value_object  = JsonObject(entry); type = json_object;  break;
                    case json_string:   value_string  = JsonString(entry); type = json_string;  break;
                    case json_boolean:  value_boolean = JsonBool(entry);   type = json_boolean; break;
                    case json_integer:  value_int     = JsonInt(entry);    type = json_integer; break;
                    case json_double:   value_double  = JsonDouble(entry); type = json_double;  break;
                    case json_array:    value_array   = JsonArray(entry);  type = json_array;   break;
                    }
                }
                else {
                    name = "INVALID";
                    type = json_none;
                }
            }
            const std::string& getName  (void) const { return name; }
            const JsonObject&  getObject(void) const { return value_object; }
            const JsonArray&   getArray (void) const { return value_array; }
            const JsonString&  getString(void) const { return value_string; }
            const JsonBool&    getBool  (void) const { return value_boolean; }
            const JsonInt&     getInt   (void) const { return value_int; }
            const JsonDouble&  getDouble(void) const { return value_double; }
            const json_type    getType  (void) const { return type; }
            std::string getValueAsString(void) const {
                switch (type) {
                case json_object:  return value_object .getValueAsString();
                case json_string:  return value_string .getValueAsString();
                case json_boolean: return value_boolean.getValueAsString();
                case json_integer: return value_int    .getValueAsString();
                case json_double:  return value_double .getValueAsString();
                case json_array:   return value_array  .getValueAsString();
                }
                return "INVALID";
            }
        };

        /// Type definition for a vector of json named value pairs
        typedef std::vector<JsonNamedValue> JsonNamedValueVector;

        /**
        * Get a vector of json named value pairs from the given json tree level.
        * @param json  pointer to a subtree in the json tree
        * @return a vector of json named value pairs, or an empty vector
        */
        static JsonNamedValueVector getNamedValues(const json_value* const json) {
            JsonNamedValueVector named_variants;
            if (json != NULL && json->type == json_object) {
                json_object_entry* elements = json->u.object.values;
                if (elements != NULL) {
                    unsigned int num_children = json->u.object.length;
                    named_variants.reserve(num_children);
                    for (unsigned int i = 0; i < num_children; ++i) {
                        named_variants.push_back(JsonNamedValue(&elements[i]));
                    }
                }
            }
            return named_variants;
        }

        typedef bool (*compare)(const std::string& lhs, const std::string& rhs);

        /**
        * Get a vector of json named value pair from the given json tree level.
        * @param elements  pointer to an array of json object entries
        * @param num_elements number of array elements
        * @param name the name of the json named value pair to search for
        * @param name_comparator a function pointer to an optional name comparater method, or NULL
        * @return a json named value pair, or an empty named value pair
        */
        static JsonNamedValue getValue(const json_object_entry* const elements, const size_t num_elements, const std::string& name, compare name_comparator) {
            if (elements != NULL) {
                for (size_t i = 0; i < num_elements; ++i) {
                    std::string element_name(elements[i].name, elements[i].name_length);
                    if (name_comparator == NULL && element_name == name ||
                        name_comparator != NULL && name_comparator(element_name, name) == true) {
                        return JsonNamedValue(&elements[i]);
                    }
                }
            }
            return JsonNamedValue(NULL);
        }

    };

}   // namespace libtasmota

#endif
