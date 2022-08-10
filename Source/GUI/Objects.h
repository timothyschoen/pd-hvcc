#pragma once

namespace hvcc
{

struct ObjectInfo
{
    using ObjectSpec = std::pair<int, int>;
    
    static ObjectSpec getObjectInfo(StringArray objectToCheck) {
        auto name = objectToCheck.getReference(0);
        objectToCheck.remove(0);
        
        if(objectTypes.count(name)) {
            return objectTypes[name];
        }
        if(specialObjects.count(name)) {
            return specialObjects[name](objectToCheck);
        }
        
        return {-1, -1};
    }
    
    inline static std::unordered_map<String, std::function<ObjectSpec(StringArray)>> specialObjects = {
        {"pack",
            [](StringArray args) -> std::pair<int, int> {
                return {std::max(args.size(), 2), 1};
            }},
        {"sel",
            [](StringArray args) -> std::pair<int, int> {
                return {1, args.size() + 1};
            }},
        {"select",
            [](StringArray args) -> std::pair<int, int> {
                return {1, args.size() + 1};
            }},
        {"unpack",
            [](StringArray args) -> std::pair<int, int> {
                if(args.size() == 0) {
                    return {1, 2};
                }
                else {
                    return {1, args.size()};
                }
            }},
        {"trigger",
            [](StringArray args) -> std::pair<int, int> {
                if(args.size() == 0) {
                    return {1, 2};
                }
                else {
                    return {1, args.size()};
                }
            }},
        {"t",
            [](StringArray args) -> std::pair<int, int> {
                if(args.size() == 0) {
                    return {1, 2};
                }
                else {
                    return {1, args.size()};
                }
                
            }},
        
        // TODO: gate, maybe more?
    };
    
    inline static std::unordered_map<String, ObjectSpec> objectTypes = {
        {"!=", {2, 1}},
        {"%", {2, 1}},
        {"&", {2, 1}},
        {"&&", {2, 1}},
        {"|", {2, 1}},
        {"||", {2, 1}},
        {"*", {2, 1}},
        {"+", {2, 1}},
        {"-", {2, 1}},
        {"/", {2, 1}},
        {"<", {2, 1}},
        {"<<", {2, 1}},
        {"<=", {2, 1}},
        {"==", {2, 1}},
        {">", {2, 1}},
        {">=", {2, 1}},
        {">>", {2, 1}},
        {"abs", {1, 1}},
        {"atan", {1, 1}},
        {"atan2", {2, 1}},
        {"b", {1, 1}},
        {"bang", {1, 1}},
        {"bendin", {0, 2}},
        {"bendout", {2, 0}},
        {"bng", {1, 1}},
        {"change", {1, 1}},
        {"clip", {3, 1}},
        {"cnv", {0, 0}},
        {"cos", {1, 1}},
        {"ctlin", {0, 3}},
        {"ctlout", {3, 0}},
        {"dbtopow", {1, 1}},
        {"dbtorms", {1, 1}},
        {"declare", {0, 0}},
        {"del", {2, 1}},
        {"delay", {2, 1}},
        {"div", {2, 1}},
        {"exp", {1, 1}},
        {"f", {2, 1}},
        {"float", {2, 1}},
        {"floatatom", {1, 1}},
        {"ftom", {1, 1}},
        {"hradio", {1, 1}},
        {"hsl", {1, 1}},
        {"i", {2, 1}},
        {"inlet", {0, 1}},
        {"int", {2, 1}},
        {"line", {3, 1}},
        {"loadbang", {0, 1}},
        {"log", {2, 1}},
        {"makenote", {3, 2}},
        {"max", {2, 1}},
        {"metro", {2, 1}},
        {"min", {2, 1}},
        {"midiin", {0, 2}},
        {"midiout", {2, 0}},
        {"mod", {2, 1}},
        {"moses", {2, 2}},
        {"mtof", {1, 1}},
        {"nbx", {1, 1}},
        {"notein", {0, 3}},
        {"noteout", {3, 0}},
        {"outlet", {1, 0}},
        {"pgmin", {2, 1}},
        {"pgmout", {0, 2}},
        {"pipe", {2, 1}},
        {"poly", {2, 3}},
        {"pow", {2, 1}},
        {"powtodb", {1, 1}},
        {"print", {1, 0}},
        {"r", {0, 1}},
        {"random", {2, 1}},
        {"receive", {0, 1}},
        {"rmstodb", {1, 1}},
        {"route", {2, 2}},
        {"s", {2, 0}},
        {"send", {0, 2}},
        {"sin", {1, 1}},
        {"spigot", {2, 1}},
        {"sqrt", {1, 1}},
        {"swap", {2, 2}},
        {"symbol", {2, 1}},
        {"symbolatom", {1, 1}},
        {"table", {0, 0}},
        {"tabread", {1, 1}},
        {"tabwrite", {2, 0}},
        {"tan", {1, 1}},
        {"tgl", {1, 1}},
        {"timer", {2, 1}},
        {"touchin", {0, 2}},
        {"touchout", {2, 0}},
        {"until", {2, 1}},
        {"vradio", {1, 1}},
        {"vsl", {1, 1}},
        {"wrap", {1, 1}},
        {"*~", {2, 1}},
        {"+~", {2, 1}},
        {"-~", {2, 1}},
        {"/~", {2, 1}},
        {"abs~", {1, 1}},
        {"adc~", {1, 2}},
        {"biquad~", {1, 1}},
        {"bp~", {3, 1}},
        {"catch~", {0, 1}},
        {"clip~", {3, 1}},
        {"cos~", {1, 1}},
        {"cpole~", {4, 2}},
        {"czero_rev~", {4, 2}},
        {"czero~", {4, 2}},
        {"dac~", {2, 0}},
        {"dbtopow~", {1, 1}},
        {"dbtorms~", {1, 1}},
        {"delread~", {1, 1}},
        {"delwrite~", {1, 0}},
        {"env~", {1, 1}},
        {"exp~", {1, 1}},
        {"ftom~", {1, 1}},
        {"hip~", {2, 1}},
        {"inlet~", {1, 1}},
        {"line~", {2, 1}},
        {"lop~", {2, 1}},
        {"max~", {2, 1}},
        {"min~", {2, 1}},
        {"mtof~", {1, 1}},
        {"noise~", {1, 1}},
        {"osc~", {2, 1}},
        {"outlet~", {1, 0}},
        {"phasor~", {2, 1}},
        {"powtodb~", {1, 1}},
        {"pow~", {2, 1}},
        {"q8_rsqrt~", {1, 1}},
        {"q8_sqrt~", {1, 1}},
        {"receive~", {1, 1}},
        {"rmstodb~", {1, 1}},
        {"rpole~", {2, 1}},
        {"rsqrt~", {1, 1}},
        {"rzero_rev~", {2, 1}},
        {"rzero~", {2, 1}},
        {"r~", {1, 1}},
        {"samphold~", {2, 1}},
        {"samplerate~", {1, 1}},
        {"send~", {1, 1}},
        {"sig~", {1, 1}},
        {"snapshot~", {1, 1}},
        {"sqrt~", {1, 1}},
        {"s~", {1, 0}},
        {"tabosc4~", {2, 1}},
        {"tabplay~", {1, 2}},
        {"tabread4~", {2, 1}},
        {"tabread~", {1, 1}},
        {"tabwrite~", {1, 0}},
        {"throw~", {1, 0}},
        {"vcf~", {3, 2}},
        {"vd~", {1, 1}},
        {"wrap~", {1, 1}}
    };
};

}
