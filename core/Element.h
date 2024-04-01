/**@file        Element.h
 * @brief       provide utils about elements' information
 * @details     all utils are provided under the namespace chem.
 *
 * @author      Xin He
 * @date        2024-04
 * @version     1.0
 * @copyright   GNU Lesser General Public License (LGPL)
 *
 *              Copyright (c) 2024 Xin He, Liu-Group
 *
 *  This software is a product of Xin's PhD research conducted by Professor Liu's
 *  Group at the College of Chemistry and Molecular Engineering, Peking University.
 *  All rights are reserved by Peking University.
 *  You should have received a copy of the GNU Lesser General Public License along
 *  with this software. If not, see <https://www.gnu.org/licenses/lgpl-3.0.en.html>
 **********************************************************************************
 * @warning    Do not include this file to any header. You'd better include it only
 *  in source files!
 * @par [logs]:
 * <table>
 * <tr><th> Date        <th> Description
 * <tr><td> 2024-03-29  <td> initial version.
 * </table>
 *
 **********************************************************************************
 */

#ifndef ELEMENTS_H
#define ELEMENTS_H
#include <cstring>
#include <string>

const int MAX_ELEMENTS_NUMBER  = 119;  // 0-th element is NU
const int MAX_ELEMENTS_NPERIOD = 8;    // 0-th period is {NU}

const int N_PURE_ElEMENTS_IN_ROW[] = {0, 2, 10, 18, 36, 54, 86, 118};  // "NU" is not a pure element

// clang-format off
const char* const ELEMENTS_LABEL[] = {
 "NU" ,
 "H" ,                                                                                "HE",
 "LI","BE",                                                  "B" ,"C" ,"N" ,"O" ,"F" ,"NE",
 "NA","MG",                                                  "AL","SI","P" ,"S" ,"CL","AR",
 "K" ,"CA","SC","TI","V" ,"CR","MN","FE","CO","NI","CU","ZN","GA","GE","AS","SE","BR","KR",
 "RB","SR","Y" ,"ZR","NB","MO","TC","RU","RH","PD","AG","CD","IN","SN","SB","TE","I" ,"XE",
 "CS","BA",
      "LA","CE","PR","ND","PM","SM","EU","GD","TB","DY","HO","ER","TM","YB", // Lanthanide
           "LU","HF","TA","W" ,"RE","OS","IR","PT","AU","HG","TL","PB","BI","PO","AT","RN",
 "FR","RA",
      "AC","TH","PA","U" ,"NP","PU","AM","CM","BK","CF","ES","FM","MD","NO", // Actinicles
           "LR","RF","DB","SG","BH","HS","MT","DS","RG","CN","NH","FL","MC","LV","TS","Og"
};

const double ELEMENTS_MASS[] = // average of isotopic atoms
{
0.0,            1.007947,       4.0026022,      6.9412,         9.0121823,
10.8117,        12.01078,       14.00672,       15.99943,       18.99840325,
20.17976,       22.989769282,   24.30506,       26.98153868,    28.08553,
30.9737622,     32.0655,        35.4532,        39.9481,        39.09831,
40.0784,        44.9559126,     47.8671,        50.94151,       51.99616,
54.9380455,     55.8452,        58.9331955,     58.69342,       63.5463,
65.4094,        69.7231,        72.641,         74.921602,      78.963,
79.9041,        83.7982,        85.46783,       87.621,         88.905852,
91.2242,        92.906382,      95.942,         98.0,           101.072,
102.905502,     106.421,        107.86822,      112.4118,       114.8183,
118.7107,       121.7601,       127.603,        126.904473,     131.2936,
132.90545192,   137.3277,       138.905477,     140.1161,       140.907652,
144.2423,       145.0,          150.362,        151.9641,       157.253,
158.925352,     162.5001,       164.930322,     167.2593,       168.934212,
173.043,        174.9671,       178.492,        180.947882,     183.841,
186.2071,       190.233,        192.2173,       195.0849,       196.9665694,
200.592,        204.38332,      207.21,         208.980401,     210.0,
210.0,          220.0,          223.0,          226.0,          227.0,
232.038062,     231.035882,     238.028913,     237.0,          244.0,
243.0,          247.0,          247.0,          251.0,          252.0,
257.0,          258.0,          259.0,          262.0,          261.0,
262.0,          266.0,          264.0,          277.0,          268.0,
271.0,          272.0,          285.0,          284.0,          289.0,
288.0,          292.0,          291.0,          294.0
};

const double ELEMENTS_MASS_NOAVG[] =
{
0.0,                1.00782503207,      4.00260325415,      7.016004548,        9.012182201,
11.009305406,       12.0000000000,      14.00307400478,     15.99491461956,     18.998403224,
19.99244017542,     22.98976928087,     23.985041699,       26.981538627,       27.97692653246,
30.973761629,       31.972070999,       34.968852682,       39.96238312251,     38.963706679,
39.962590983,       44.955911909,       47.947946281,       50.943959507,       51.940507472,
54.938045141,       55.934937475,       58.933195048,       57.935342907,       62.929597474,
63.929142222,       68.925573587,       73.921177767,       74.921596478,       79.916521271,
78.918337087,       85.910610729,       84.911789737,       87.905612124,       88.905848295,
89.904704416,       92.906378058,       97.905408169,       98.906254747,       101.904349312,
102.905504292,      105.903485715,      106.90509682,       113.90335854,       114.903878484,
119.902194676,      120.903815686,      129.906224399,      126.904472681,      131.904153457,
132.905451932,      137.905247237,      138.906353267,      139.905438706,      140.907652769,
141.907723297,      144.912749023,      151.919732425,      152.921230339,      157.924103912,
158.925346757,      163.929174751,      164.93032207,       165.930293061,      168.93421325,
173.938862089,      174.940771819,      179.946549953,      180.947995763,      183.950931188,
186.955753109,      191.96148069,       192.96292643,       194.964791134,      196.966568662,
201.970643011,      204.974427541,      207.976652071,      208.980398734,      208.982430435,
210.987496271,      222.017577738,      222.01755173,       228.031070292,      227.027752127,
232.038055325,      231.03588399,       238.050788247,      237.048173444,      242.058742611,
243.06138108,       247.07035354,       247.07030708,       251.079586788,      252.082978512,
257.095104724,      258.098431319,      255.093241131,      260.105504,         263.112547,
255.107398,         259.114500,         262.122892,         263.128558,         265.136151,
281.162061,         272.153615,         283.171792,         283.176451,         285.183698,
287.191186,         292.199786,         291.206564,         293.214670
};
// clang-format on

namespace Elements {

struct ElemInfo {
    int         znum;
    int         fmly;
    int         perd;
    double      mass;
    std::string name;
};

inline int ELEMENTS_ZNUM(const std::string& label) {
    std::string copylabel = label;
    std::for_each(copylabel.begin(), copylabel.end(), [](char& c) { c = ::toupper(c); });
    for (int i = 1; i <= MAX_ELEMENTS_NUMBER; ++i) {
        if (strcmp(copylabel.c_str(), ELEMENTS_LABEL[i]) == 0) return i;
    }
    return 0;
}

inline std::string ELEMENTS_NAME(const int& znumber) {
    return (znumber > 0 && znumber <= MAX_ELEMENTS_NUMBER) ? std::string(ELEMENTS_LABEL[znumber]) : "NULL";
}

template <typename T>
ElemInfo& GetElemInfo(T ask) {
    ElemInfo elem;
    if (std::is_same<T, std::string>::value) {
        elem.name = ask;
        elem.znum = ELEMENTS_ZNUM(elem.name);
    } else if (std::is_same<T, int>::value) {
        elem.znum = ask;
        elem.name = ELEMENTS_NAME(elem.znum);
    }
    elem.perd = 0, elem.fmly = 0;
    for (int ip = 1; ip <= MAX_ELEMENTS_NPERIOD; ++ip) {
        if (elem.znum <= N_PURE_ElEMENTS_IN_ROW[ip - 1] && elem.znum <= N_PURE_ElEMENTS_IN_ROW[ip]) {
            elem.perd = ip, elem.fmly = elem.znum - N_PURE_ElEMENTS_IN_ROW[ip - 1];
        }
    }
    elem.mass = ELEMENTS_MASS[elem.znum];
    return elem;
}

};  // namespace Elements


#endif  // ELEMENTS_H
