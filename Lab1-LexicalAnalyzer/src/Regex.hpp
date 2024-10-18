#include<string>
#include<deque>
#include<iostream>
#include<stdexcept>
#include<cstring>
#include<memory>
#include<unordered_map>

static char SEPARATOR = '|'; 
static char KLEEN = '*'; 
static char JOIN = 0; //znak koji odvaja simbole u nizu, 0 označava nepostojeći znak, Regex tada smatra svaki znak zasebnim simbolom
static char BRA = '(', KET = ')'; //obićne zagrade za regularne izraze
static char INCL_BEGIN = '{', INCL_END = '}'; //zagrade za oznaku referenci
static char BLANK = '$';

/* VILIMOV ZAKON REGEX API ZA C++
    Regex je wrapper za stringove koji na temelju zadanih posebnih znakova (iznad ^^^^) parsira string u segmente 
    i omogućava efikasno interpretiranje regularnog izraza.
    Regex zauzima jednaki memorijski prostor kao string koji wrappa, a ako je konstruiran iz konstante, ne zauzima nikakav prostor!
    Konstrukcija se izvodi s vremenskom složenosti reda veličine O(n*logn) u prosjeku, a vjerujte mi da brže ne može!
    Nadalje, Regex automatski optimizira izraz tako da je čitanje još brže! (Optimizirani string može se dobiti i metodom reduce()).
    Regex se djeli na segmente (njegovu djecu) koja se po potrebi dalje djele dok u listovima ne ostanu samo atomični segmenti.
    Implementiran je jednostavan foreach iterator za prolaženje po djeci pojedinog segmenta, a tip (koji je detaljnije objašnjen iznad ^^^^) 
    se provjerava funkciom type().
    Druge korisne funkcije opisane su unutar klase, a one za javnu uporabu su dodatno itaknute.
    Detalji o implementaciji nalaze se u Regex.cpp.

    Hvala što ste odabrali moj API, neka vam je sa srećom!
*/
class Regex {
public:
    static std::unordered_map<std::string, Regex> saved;
    enum Type {
        HAS_SEPARATOR, /*
            Oznacava da regex ima separator '|'
        */
        HAS_JOIN, /*
            Oznacava da je regex povezan niz regexa
        */
        ATOMIC /*
            Oznacava da regex nema djece
        */
    };

private:
    char* exp; //pokazivać na regularni izraz
    size_t size; //duljina regularnog izraza
    
    std::deque<Regex> subdivisions; //particije tj. djeca izraza
    Type regex_type = HAS_SEPARATOR; //tip izraza
    int collapsed = 0; //brojać ugnježđenih zagrada
    
    //zastavice
    bool kleen;
    bool is_root = false;
    bool is_str_const;

public:
    //default konstruktor
    Regex();
    //copy konstruktor
    Regex(const Regex& r);
    //move konstruktor
    Regex(Regex&& r) noexcept;
    /*konstruktor pomoću stringa
        praznine se smatraju znakovima, a neispravno postavljene zagrade bacaju error! 
    */
    Regex(const std::string& str);
    //konstruktor pomocu string konstante, koristi strlen() (nema root niti zauzima memoriju POG!)
    Regex(const char* str);
    //destruktor se poziva samo za root element (zastavica is_root)
    ~Regex();

    //konstruktor za djecu NE KORISTITI!
    Regex(char* str, size_t size, bool is_str_const = false, Type type = HAS_SEPARATOR);

    //---------JAVNE METODE------------//

    //assignment operatori
    void operator= (Regex&& r) noexcept;
    void operator= (const Regex& r);
    void operator= (const std::string& r);
    void operator= (const char* str);

    //sprema regex (koristiti include zagrade)
    void save_as (const std::string& name);
    
    //vraća spremljeni regex
    static const Regex& open (const std::string& name);

    //vraca deliminator, NE SMIJE SE ZVATI ZA ATOMIC TIP!
    char deliminator() const;
    
    //overload za konverziju u string
    operator std::string() const;
    
    //vraca pojednostavljeni regex
    std::string reduce() const;

    //radi samo za ATOMIC tip, vraća sadržani simbol
    char get() const;
    
    //overload za ispis regexa
    friend std::ostream &operator<< (std::ostream& os, const Regex& r);
    
    //vraca tip regexa
    Type type () const;
    
    //vraca ima li regex kleen operator
    bool has_kleen() const;
    
    //funkcije za iteriranje po djeci regexa
    std::deque<Regex>::const_iterator begin () const;
    std::deque<Regex>::const_iterator end () const;

    //---------JAVNE METODE-----------//

private:
    void assertType();
    static bool vectorize_string (std::deque<Regex>& v, char* str, size_t size, char deliminator = 0);
};

/*printanje regexa
    OR:[] označava niz regexa odvojenih separatorom '|'
    AND:[] označava niz regexa spojenih s prazninom
    ostali izrazi su atomični regexi: 
        pojedinaćni znakovi
        reference na druge regexe obavijene s '{' '}'
    (*) može se pojaviti na bilo kojem od gore navedenih izraza

    poziv funkcije: printRegex(const Regex& reg);

    (Regexi automatski pojednostavljuju izraze i miću prazne znakove vidi primjer u Main.cpp)*
*/
namespace print_util {

    //brojac
    static int TABS = 0;

    //base tab, može se redefinirati po volji
    static std::string TAB = " ";

    //funkcija za generiranje uvlaka
    static std::string tab(int n) {
        std::string _tab = "";
        for (int i=0; i<n; i++) _tab += TAB;
        return _tab;
    };

    //makro definicije za ispis
    #define M_KLEEN (std::string) (r.has_kleen() ? "*" : "")
    #define M_TYPE (std::string) (r.type() == Regex::HAS_JOIN ? "AND" : "OR") + M_KLEEN 

    static void printRegex(const Regex& r) {
        std::cout <<tab(TABS);
        if (r.type() != Regex::ATOMIC) {
            std::cout <<M_TYPE <<":[\n";
            TABS++;
            for (auto it = r.begin(); it != r.end(); it++) {
                printRegex(*it);
                std::cout <<"\n";
            }
            TABS--;
            std::cout <<tab(TABS) <<"]:" <<M_TYPE;
        } else 
            std::cout <<r <<M_KLEEN;
    }
}