#include "logger.h"
#include<stdio.h>
#include<sys/types.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<termios.h>
#include <cstring>


void chery(){
    std::string BigChery[] = {   "                           EjtfGDEGGf,:ft                   ", // 60
                            "                      ,tLjfDfGGLEEEKKEKEKEG.                ",
                            "                    ,,iLLLGLDEDDEEKKKKKKKKWE                ",
                            "                  :;i;tLLLEDEEEEEKKKKKKEDKKKL.              ",
                            "                 :;iitjLLGDGEDEKKKKKKKKEEWEKDD.             ",
                            "              :.:,;ttjjffDDDGDEKKKKKKKKKKKEKEEDL.           ",
                            "             ;.:ittffffLLLGDEEEEKKKEKKKWKKKKKKEEDf          ",
                            "             :;tjjfDDGLLGEDDLDDEKKEEKKKWKKKKKKKKEED         ",
                            "           tiiLfLGEKEDDEEEDLGDEKKEEEKKKKKKKKKKKWKEKL        ",
                            "           jifGfGDKKEEDDDELGDDEKKEEEKKKKKKKKKKKKEEKD        ",
                            "          :ijGfGEKKEEEDDGDEEDDEEKEKEKKEEKKKKEKKDDKKKE       ",
                            "          ifLLDKKKEKKKDEEKEEDKDKEEKKKEEEKKWKKWEDDKKEK:      ",
                            "         :iGDDEKKKKKKEEKKKKEEEEKEKKKKEEEEKKKKWEDDKKKKE      ",
                            "         ,fEDEKKKKKKKKKKKKKKKKKKKKKEKKKEEKDKKWKEEKEKEKD     ",
                            "         iGEEKKKKKKWWWWKKKKWKWWKKKKKKKKKKEEKKWKEKKEKEEE     ",
                            "        :jEEKKKKKKWWWWWWWWWWWWWWKKKKWKKKKEEKKWKKKEKKKKK.    ",
                            "        tLEEKKKKKWWWWWWWWWWWWWKKKKKKKWWWKKKKKKKKKKKKKKKEL   ",
                            "        ;DKKKKKKWWWWWWWWWWWWKKKWKDKKKWWWWKKKKKWWWKKKKKKEE   ",
                            "       jjEKKKKWWWWWWWWWWWWWKKEKKDDKKKWWWWWKWKKWWWKKKKKKKE   ",
                            "       jjEKKKKWWWWWWWWWWWWKKEEKKLDEKKWWWWWWWKWWWWWWKKKKKE   ",
                            "       ,LKKKKWWWWWWWWWWWWKKEDDELLDDDKWWWWWWWKKWWWWWKWKKK;   ",
                            "       ,DKKKKWWWWWWWWWWWKKDGGDLjfGfGEKWWWWWWWKWWWWWWWKKK    ",
                            "       :DKKKKWWWWWWWWWKKEGLffftitjjfGKKWWWWWWWKWWWWKKKWE    ",
                            "       .EKKKKWWWKKKKKKEDGti;;;,;iiitfDKWWWWWWWWWWWWKKKKK    ",
                            "        EKKKKWWKKKKKEGLji,,::::,,,;tjGKWWWWWWWWWWWWWWWWE    ",
                            "        KKKEKKKEDGGLLti;,,:::::::,,;ifDKKWWWWWWWWWWWWWW     ",
                            "         KELEEDGjttt;;,,,::::..::,,;;tfGEWWWWWWWWWWWWWE     ",
                            "        ;GLjGLjiii;;;;,,,,,,,,:,::,,;itfGKWWWWWWWWWWWK;     ",
                            "        :jjtLfj;;;;;;;;,,,,,;;;;,,,,,itjLEWWWWWWWWWWWWL     ",
                            "        .tjfGDDLtiiiiiiittfLLLffjti;;;itfGKWWWWWWWWWWW      ",
                            "        :LtjjfGEDLjtitfGGGGGLfjjjfttiiitjfEWWWWWWWWWWW      ",
                            "        LE;;;itjGGDfijLGGGLji;,;ijjtttiitjDKWW#W#WW#WK      ",
                            "       tjL,;iitjLGLfiifLLLftii;;;ittjjjttjLEWWKtiL##WLt     ",
                            "     :fiiLDfGGDEKELt,:DEKKEEEEEt.:jLfjfjfLfKKEGLEWKGfi.     ",
                            "     fLt. :iGLDKKWWLEWjLLGDEKKWGtDDjfEEfEDGEKWW#WEGEf;      ",
                            "     LWjL:i;ijfGLfWKWDfttjEGfKKKKEKWWWWELLfDWW#WWEDEEt      ",
                            "      #:..j,;tffjjKfjWttttfLLLGGGjD#WfjtffLGWWWKGELED;      ",
                            "      Wi fG,iijjttG;iKKDitjLLLLffjGGtttjjjfGKWWGKGfDj       ",
                            "      KL iG,;tttiff,,GELjitffffjjjEjiittjjfLKWEEWDfDi       ",
                            "      ;K::t;itt;tK,.,tGjitttffjjtiK;iittjjjLKKjWKEfD        ",
                            "       WG;:,i;fGGt..:,jLGLjjjjtijfGtittttjjLEfjDEDfW        ",
                            "        WWG;iiiit....,ijjtffLLLftittiittjjfLLtjEDfE         ",
                            "         ;W,;,;ii....:;tjfjtiititttttttttjjjttfifi#         ",
                            "          #,,;;t,::::,,;ijftiiiiiitttttttjjttttfjK          ",
                            "          W:,,;ti;;;;;;;;jLji;;;;;itjttttjjtti;;if          ",
                            "           ::;;tttttfGDtijLjii;;;;ittttjjjjt;;iiE           ",
                            "           ,:;ititjGDDDDGLfjt;;;;iitttjttjjtjt;K            ",
                            "           .,,iii;ijfLffftiiti;;;iitttttttjtfW              ",
                            "            .;i;i;;tttttiiiitii;iiitttitjjjtfW              ",
                            "            ;;i;;;ii;;;;;;;iiiiiiiitttttjjjffLK,            ",
                            "             ;i;;jji;i;,,;;iiiii;iiiititjfjjLKEE#           ",
                            "             ;;;LLjtjjfti;;;iiiiiiittttjjfjLLWfK#           ",
                            "             ,iLjitLLfGEDjiitiiiiitttttjjfjGKL:fW.          ",
                            "             ,ii;iiii;iitGEftti;itttttjfjjfDD ::,#          ",
                            "              i;;tttttjjjjjLGjiiittttttfjffD:..::L          ",
                            "              .;itjjfjjttttjjtiittttjjfffEG:,.....#         ",
                            "               ti;i;ittttttititttttjjffGEj.:::.. .W         ",
                            "               Ki;iiiitttttttjttjjjfffGEE:... ;...D,        ",
                            "               ;iiiitittjjtttjttjjjffGEDG... .:...;#        ",
                            "                ;;iijttttttiittjjffGDKDD:..... ...:,,       ",
                            "                ,,jtttjjttiittjjjLEEEEL..:. .... ...;:      ",
                            "                , .ijffjjtttttjLDEEDEf .. :.... . .; G      ",
                            "               . : ;t;iitttjfGDDDEDGG; ... ...  ..::,.G     ",
                            "                . .  ,f;ifffLGGGGGE;:.  ....    .  .:...    ",
                            "                 .,. ij,itjffLLLLL,:i:.. . ... . ..  .  .   ",
                            "              .     ..f;;itttjfjL;....::. . . .:t,.    . .  ",
                            "             . .   ...:,,ii;iitti;... .::  .,.:..:     . .. ",
                            "              :     ..:,,;i;;iit:...   .:.  .:.         ..;D",
                            "           .   .     .,:,;,,,,t,..  .  .,   ..:.    .   ..iL",
                            "                    ..;.:,,:,,, . ..    .,..i.:..  . .  ::jD",
                            "         .  :.    ....j:::::::. ..         ,.. ::; .    .ifL",
                            "                .....:j:::..:i.   .       .       .      ;LG",
                            "                 ,.:.:i:::..  .                         .;iD",
                            "       t  .     .L.::;;:...  .,     .:      ..      ..  :tGE",
                            "        ,  :t   :...:f;..    :.:.     ,:.              .,ffE",
                            "      :;;. ,. : ; ..:,        .. .   .       ....   . ..:iDK",
                            "     j.Li        ...;..   ;.  ...     ..      :.... ...:tLEK"    };
 std::string SmallChery[] = {
                            "                              ", // 30
                            "           iLLGLEEEEKE        ",
                            "        .i;jGDDEKKKEKKG       ",
                            "       .;ttjGGDKKKKKKKKK.     ",
                            "      ,jjDDDEDDEKEKWKKKWEL    ",
                            "     iLfDKEGDDDEKEKEKKKKEK    ",
                            "     tGKKKKEKKEKEWEKKKWEEKK   ",
                            "    :EEKKKWKKKKKKKKKKEKEKEE.  ",
                            "    tEKKKWWWWWWWKKWWKKKKKKKE  ",
                            "    GKKKWWWWWWKKDKWWWKKWKKKK  ",
                            "    DKKWWWWWWKEKLEWWWWKWWWKK  ",
                            "    EKWWWWWWEGGtffEWWWWWWWWW  ",
                            "    KKWWKKKGt,,,;;GWWWWWWWWE  ",
                            "    KDEEfji,,::.:,tDWWWWWWK   ",
                            "    EiLti;;,,,;;,,;tDWWWWWK   ",
                            "    jfGEfiijLDGfjt;ijKW#WWW   ",
                            "    L,itGGiGGj;,itjttDWWKWW   ",
                            "   j,;fEKLiDGLLjt;GfEDKW#Gi   ",
                            "   t ;tLfGKjtLGDDK#LfGWWEDG   ",
                            "   K.;iti,jEijffjKttjfWGKL.   ",
                            "   W::;iK.,GttjttfittfKfEL    ",
                            "    ;;i;;.:tjjjtttitjjtLff    ",
                            "     G,i,,,;tji;itjttjtij     ",
                            "     W,ttfDDffi;;tttjjji      ",
                            "      ,i;tffjit;ittttjL       ",
                            "      ,;,;;;;;iiiiitjjEf      ",
                            "       ifjjGt;iiittjfjWiW     ",
                            "       ;ittjjEjitttjjL..i     ",
                            "       t;ijjtttittjfE.:..     ",
                            "        iiitttjtjffEj ::.#    ",
                            "        ,itjtitjfDEGi. . ::   ",
                            "        :.itttjGEEG..... ::   ",
                            "         .ittjfLL,,...    ..  ",
                            "       . ..jtitft..:.   ;...  ",
                            "     ,     t;;;t . .: :     .f",
                            "        .  :,:: ..   fj,.   :L",
                            "         .::::..  .  .   .  tf",
                            "   j    j.::. .:  . . .  .. ,D",
                            "   j:.   .:: ....  :   ... .tE"};
    std::string SquareChery[] = {
                            "                     .,jGLGGLDDLEEEKKKKKWKEi                ",
                            "                 .,iiitfLGDDEDEKKKKKKKKEDKKKDt              ",
                            "             .:.,iijjjjffGGDDDKKKKKKKKKKKKKKKEEDj.          ",
                            "           ..,jjjfGEEDDDDEDGGDDKKKEEKKKWKKKKKKKWKKE:        ",
                            "          :tjGLGEKKKEEDGGDDEDEEEKEKEKKEEKKKKEKKDDKKKG       ",
                            "         .tGDDEKKKKKKEEKKKKKKEEKEKKKKEEEEKEKKWEDEKKKKK      ",
                            "         tDEEKKKKKWWWWWWWWKWWWWKKKKKKKKKKEEKKWKEKKEKKEK     ",
                            "        iGEKKKKKKWWWWWWWWWWWWKKKKKKKKWWWKKKKKKKWKKKKKKKKL   ",
                            "        iDKKKKKKWWWWWWWWWWWWKKKWKDKKKWWWWWKWKKWWWKKKKKKKD   ",
                            "       ;LEKKKWWWWWWWWWWWWWKEDDKDLEDEKWWWWWWWKKWWWWWKKKKKj   ",
                            "       :DKKKKWWWWWWWWWKKEDLffftijfjfGKKWWWWWWWKWWWWWKKWK    ",
                            "       .KKKKKWWKKKKKKDLjt,,::::,;,;tjGKWWWWWWWWWWWWWWWWD    ",
                            "        ,WEGDEDGjttti,,,,::::...:,,;;tfGEWWWWWWWWWWWWWD     ",
                            "        .tjjLLfti;;;;;;;;;itjjjti;;;;;ijfDKWWWWWWWWWWW;     ",
                            "        tDiiiijLDGGjijLGGGGfjiiitjjttiiitjDKWWW###W#WW      ",
                            "     .ittfttjfLGGDLt,;fGDDGGLLfi;;tjjjjjjffKKKEijDWEDt;     ",
                            "     jKti:iijfGEDGWK#EtjjLEDGWWEEDKKW#WDLLLEWW##WEDEDi      ",
                            "      W, jG,;tjjtjDiiKDLtijLLfLfftGEjittjjfGWWWGDGfDL       ",
                            "      EG.;L,;ttt;Gj:,LEftitjfffjjtEjiittjjfLKWGKWDLG;       ",
                            "       fKLiittjff;..:,tffLfffjjtjLjtittttjfLDjtEEGLG        ",
                            "         :W,,;;t;:...:,itfjtiititttttttttjjjtjjtjfL         ",
                            "          j,:;;ttiiiiti;;jLji;;;;;itjtttjjjtiii;L,          ",
                            "           .:;iiiitLDGGDGfttt;;;;iittttttjjtjG.,            ",
                            "            ,;i;;;ii;;;;;;;ii;iiiiiiitttjjjjfGLt.           ",
                            "             ,;iLLfjffLjt;;;iiiiiiittttjjfjLGWjK#           ",
                            "             :ii;iiii;itjGDGjtiiitttttjjjjLEf :.,#          ",
                            "              .;iitjjjjtttttjtiittttjjffLEt::.....K         ",
                            "               fiiiitittttttttttjjjffLDEG.....,...fL        ",
                            "                ;iiijttttttitttjjjfGDKKG;.   .....:f,       ",
                            "                ;.:ijffjtttittjLDEKEEL.  .:...... .; G      ",
                            "                 ....if;ijjfLGGGGGDt;..... .    . ..:.:.    ",
                            "              .     ..t,;ittjjfjGi ...:: .. ...:i,.    ...  ",
                            "              .      .:,,;;;;;ii...     :.  ...         ..;D",
                            "            .      ...t:::::::: ...      ..,i:,..: .    .;jG",
                            "                 :...:t:::.....           .             .;jG",
                            "       :.  :,   ,;.::f;...   ,.:.    .:.    .      .    :jfE",
                            "     ,.Lt     .  ...,     ,   ..      ..      :........:ifEK"};
    std::string SquareSmallChery[] = {
                            "                t;ffLGEDftLt;           ",
                            "           .,iijLDDDEKKKKKEKKEt         ",
                            "         ::iijjfLGGDKEKKKKKKKKEK,       ",
                            "       ,tGLDKEEDDLDDEKEEKKKKKKKKEK;     ",
                            "      .fDEKKKKEEKKEEEKKKKEEKEKWEEKKK    ",
                            "      fEKKKKWWWWWWWWWWKKWKKEKKKKKKKKK,  ",
                            "     ;DKKKWWWWWWWWKKKWDKWWKWKKKWKKKKKK  ",
                            "     tKKWWWWWWWWKEGDGjGLKWWWWWKWWWWKKE  ",
                            "     fKKKWKKKKDf;,:::;,;LKWWWWWWWWWWW:  ",
                            "      GiLji;;;;,,::::,:,;tLKWWWWWWWWt   ",
                            "     :DiijGGGijGGGfiitjtiitjDKW##WWW    ",
                            "   .itftjLGDL;;GDGGLf;;jjjjffKKfLWDt;   ",
                            "   ,W,.;iffjWjKitLLGDGGWKLjffEWWDGK;    ",
                            "    ,L.,ittE:.iLttjjtttLittjjDELEjL     ",
                            "      :t;;t:..:;jjtititttttjjjjitL      ",
                            "       E,;jiiitiiLt;;;;tjtjjjti;E       ",
                            "        ,ii;;tttt;tii;iitttjjjE         ",
                            "        :;jLffff;;iiiiitttjfjGWtt       ",
                            "         i;tjjjjjjGjiitttjfLDi..::      ",
                            "          LiiiitttttttjffDK....: f      ",
                            "           ,tttjtiitjjLEKG.:.. . .,,    ",
                            "           ...i;ijfGGGGL:... .   .:.:   ",
                            "         .   ..iiiiitf:. .:  .. :.    .,",
                            "        . .  ..,,::::...   ..,::.     ;L",
                            "           :..:::....       .         ;f",
                            "    .;.. .. ..f.   ...   ..  , .     :iE"};
    std::string ShuYuMo[] = {
        "  ______   __             ____  ____        ____    ____         ",
        ".' ____ \\ [  |           |_  _||_  _|      |_   \\  /   _|        ",
        "| (___ \\_| | |--.  __   _  \\ \\  / /__   _    |   \\/   |   .--.   ",
        " _.____`.  | .-. |[  | | |  \\ \\/ /[  | | |   | |\\  /| | / .'`\\ \\ ",
        "| \\____) | | | | | | \\_/ |, _|  |_ | \\_/ |, _| |_\\/_| |_| \\__. | ",
        " \\______.'[___]|__]'.__.'_/|______|'.__.'_/|_____||_____|'.__.'  ",
        "                                                                 ", };
    struct winsize size;
    ioctl(STDIN_FILENO,TIOCGWINSZ,&size);
    if(size.ws_col >= 150) {
        for(int i = 0; i < static_cast<int>(sizeof(SquareChery) / sizeof(std::string)); i++) {
            Logger(std::cout, INFO, "Chery") << SquareChery[i] << kkl();
        }
    } else {
        for(int i = 0; i < static_cast<int>(sizeof(SquareSmallChery) / sizeof(std::string)); i++) {
            Logger(std::cout, INFO, "Chery") << SquareSmallChery[i] << kkl();
        }
    }
    Logger(std::cout, INFO, "And And And") << kkl();
    Logger(std::cout, INFO, "And And And") << "============================== and ==============================" << kkl();
    Logger(std::cout, INFO, "And And And") << kkl();
    for(int i = 0; i < static_cast<int>(sizeof(ShuYuMo) / sizeof(std::string)); i++) {
            Logger(std::cout, INFO, "ShuYuMo") << ShuYuMo[i] << kkl();
        }
    return ;
}


