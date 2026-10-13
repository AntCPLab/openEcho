/*
 * ABFLShare.h
 *
 */

 #ifndef PROTOCOLS_ABFLSHARE_H_
 #define PROTOCOLS_ABFLSHARE_H_
 
 #include "Rep3Share.h"
 #include "SemiShare.h"
 #include "ABFL.hpp"
 #include "GC/ABFLSecret.h"
 #include "Protocols/ABFLMC.hpp"
 
 namespace GC {
     class ABFLSecret;
 }
 
 template<class T>
 class ABFLShare : public Rep3Share<T>
 {
     typedef Rep3Share<T> super;
     typedef ABFLShare This;
 
 public:
     typedef ABFL<ABFLShare<T>> Protocol;
     typedef ABFLMC<ABFLShare<T>> MAC_Check;
     typedef MAC_Check Direct_MC;
     typedef ReplicatedInput<ABFLShare<T>> Input;
     typedef SpecificPrivateOutput<ABFLShare> PrivateOutput;
     typedef ReplicatedPrep<ABFLShare> LivePrep;
     typedef Rep3Share<T> Honest;
     typedef T random_type;
     typedef This Scalar;
 
     typedef GC::ABFLSecret bit_type;
 
     // indicate security relevance of field size
     typedef T mac_key_type;
 
     const static bool expensive = true;
     static const bool has_trunc_pr = false;
     static const bool malicious = true;
 
     static string type_short()
     {
         return "E" + string(1, T::type_char());
     }
     static string type_string()
     {
         return "errorpg " + super::type_string();
     }
 
     ABFLShare()
     {
     }
     ABFLShare(const T& other, int my_num, T alphai = {}) :
             super(other, my_num, alphai)
     {
     }
     template<class U>
     ABFLShare(const U& other) : super(other)
     {
     }
 
     static ABFLShare get_zero() {
         ABFLShare res;
         res.assign_zero();
         return res;
     }
 };
 
 #endif /* PROTOCOLS_MALICIOUSREP3SHARE_H_ */
 