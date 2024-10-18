#include"Regex.hpp"

//default konstruktor
Regex::Regex() {}
 
//copy konstruktor
Regex::Regex(const Regex& r) {
    *this = r;
}

//move konstruktor
Regex::Regex(Regex&& r) noexcept {
    *this = std::move(r);
    r.exp = nullptr;
}

//konstruktor pomoću stringa
Regex::Regex(const std::string& str) {
    *this = str;
}

void Regex::operator= (const Regex& r) {
    *this = (std::string) r;
    is_str_const = r.is_str_const;
}

void Regex::operator= (Regex&& r) noexcept {
    is_str_const = r.is_str_const;
    is_root = r.is_root;
    collapsed = r.collapsed;
    kleen = r.kleen;
    regex_type = r.regex_type;
    exp = r.exp;
    size = r.size;
    r.is_root = false;
    r.exp = nullptr;
    std::deque<Regex> _temp = std::move(r.subdivisions);
    subdivisions.clear();
    subdivisions = std::move(_temp);
} 
void Regex::operator= (const std::string& str) {
    if (is_root) delete[] (exp - collapsed); 
    regex_type = HAS_SEPARATOR;
    collapsed = 0;
    is_str_const = true;
    kleen = false;
    subdivisions.clear();
    exp = new char[str.size()+1];
    str.copy(exp, str.size());
    exp[str.size()] = '\0';
    size = strlen(exp);
    assertType();
    is_root = true;
}

void Regex::operator= (const char* str) {
    if (is_root) delete[] (exp - collapsed); 
    regex_type = HAS_SEPARATOR;
    collapsed = 0;
    is_str_const = true;
    is_root = false;
    kleen = false;
    subdivisions.clear();
    exp = (char*) str;
    size = strlen(exp);
    assertType();
}

//konstruktor pomoću string konstante
Regex::Regex(const char* str) : Regex((char*) str, strlen(str)) 
{
    is_str_const = true;
}

//konstruktor za djecu (nije za uporabu)
Regex::Regex(char* str, size_t size, bool is_str_const, Type type) 
    : is_root(false), size(size), exp(str), regex_type(type), is_str_const(is_str_const)
{
    if (size <= 0) {
        size = 0;
        return;
    }
    assertType();
}

//destruktor se poziva samo za root regex jer ostali djele isti izraz
Regex::~Regex() {
    if (is_root) delete[] (exp - collapsed); //provjera za poseban slucaj kada je originalni izraz okružen zagradama
}

/* opis:
    Određuje tip regexa.
    Započinje s pretpostavkom da ima separatore, ako ne uspije pronaći ih traži spajanja, 
    ako ne pronađe ništa znaći da se radi ili o zagrađenom izrazu ili o znaku.
    Zagrađeni izrazi se razrješavaju i kolapsiraju u čisti izraz.
    Dodatno se provjerava postojanje kleen operatora.
*/
void Regex::assertType() {
    switch (regex_type)
    {
    case HAS_SEPARATOR:
        regex_type = HAS_SEPARATOR;
        if (vectorize_string(subdivisions, exp, size, SEPARATOR)) //ako ne uspije prijelazi na sljedeći switch case
            break;
    case HAS_JOIN:
        regex_type = HAS_JOIN;
        if (vectorize_string(subdivisions, exp, size, JOIN)) // --//--
            break;
    case ATOMIC:
        regex_type = ATOMIC;
        while (exp[0] == BLANK || exp[0] == SEPARATOR && size > 0) {
            exp++;
            size--;
        } 
        if (!size) return;
        if (exp[size-1] == KLEEN) { //kolapsiraj sve kleenove u jedan ako ih ima
            while (exp[size-1] == KLEEN) size--;
            kleen = true;
        }
        if (exp[0] == INCL_BEGIN) {
            std::string str = std::string(exp).substr(0, size);
            subdivisions.emplace_back(open(str));
        }
        else if (exp[0] == BRA) //provjeri postoji li nested regex
            subdivisions.emplace_back(exp+1, size-2, false, HAS_SEPARATOR);
        else return; //inaće gotov assertion
        if (!subdivisions.back().size) //ako je nested regex prazan onda si i ti
            subdivisions.pop_back();
        else { //inaće preuzmi resurse nested regexa
            collapsed += 1;
            bool _kleen = kleen;
            bool is_str = is_str_const;
            int _collapsed = collapsed;
            *this = std::move(subdivisions.back());
            is_str_const = is_str;
            kleen |= _kleen;
            collapsed += _collapsed;
            return;
        }
    }
    if (subdivisions.empty()) size = 0, kleen = false; //ako nije ATOMIC i nema djece prazan je
}

/* opis:
    U subdivisions spremi djecu dobivenu separiranjem po '|' ili spajanjem po praznini
    ako ne uspije pronaći traženi delimiter vraca false inaće true
*/
bool Regex::vectorize_string (std::deque<Regex>& v, char* str, size_t size, char deliminator) {
    int bracket_count = 0; bool include = 0;
    bool has_delimiter = false;
    size_t len = 0;
    for (size_t p = 0; p < size; ++p) 
    {
        //provjera include izraza { }
        if (str[p] == INCL_BEGIN) {
            if (include) 
                throw std::invalid_argument("improper include statement");
            include = true;
        }
        if (str[p] == INCL_END) {
            if (!include) 
                throw std::invalid_argument("improper include placement");
            include = false;
        }
        //provjera zagrada ( )
        if (str[p] == BRA) bracket_count++;
        if (str[p] == KET) bracket_count--;
        if (bracket_count < 0) 
            throw std::invalid_argument("improper bracket placement");
        
        if (bracket_count + include) continue; //ignoriraju se izrazi unutar zagrada i include izraza

        if (str[p] == BLANK) {
            if (!deliminator) len++;
            continue;
        }

        //prepoznavanje kleen operatora kada je delimiter prazan znak
        bool noDelimiter = !deliminator && p != size-1 && str[p] != BLANK;
        if (noDelimiter) noDelimiter = str[p+1] != KLEEN;
        if (noDelimiter || str[p] == deliminator) {
            has_delimiter = true;
            if (p-len+noDelimiter > 0) { //rješava slučajeve zaredanih separatora |||...
                v.emplace_back(
                    str+len, p-len+noDelimiter, false,
                    deliminator == SEPARATOR ? HAS_JOIN : ATOMIC
                );
                if (!v.back().size) //rješava slučajeve prazne djece ()
                    v.pop_back();
                else if (v.back().regex_type != ATOMIC && v.back().deliminator() == deliminator && !v.back().kleen) 
                { //rjesšva se nepotrebnih zagraba izmedu operacija istog tipa
                    Regex _temp = std::move(v.back());
                    v.pop_back();
                    while (!_temp.subdivisions.empty()) {
                        v.emplace_back(std::move(_temp.subdivisions.front()));
                        _temp.subdivisions.pop_front();
                    }
                    _temp.is_root = false;
                }
            }
            len = p + 1;
        }
    }
    if (bracket_count) throw std::invalid_argument("improper bracket placement");
    if (include) throw std::invalid_argument("improper include statement");
    
    if (size-len > 0 && v.size()) 
    {
        v.emplace_back(
            str+len, size-len, false,
            deliminator == SEPARATOR ? HAS_JOIN : ATOMIC
        );
        if (!v.back().size)
            v.pop_back();
        else if (v.back().regex_type != ATOMIC && v.back().deliminator() == deliminator && !v.back().kleen) 
        {
            Regex _temp = std::move(v.back());
            v.pop_back();
            while (!_temp.subdivisions.empty()) {
                v.emplace_back(std::move(_temp.subdivisions.front()));
                _temp.subdivisions.pop_front();
            }
        }
    }
    return has_delimiter;
}


//vraca deliminator, ne smije biti pozvan za ATOMIC tip!
char Regex::deliminator() const {
    switch (regex_type)
    {
    case HAS_JOIN:
        return JOIN;
    case HAS_SEPARATOR:
        return SEPARATOR;
    default:
        throw std::invalid_argument("Atomic regex has no deliminator!"); 
    }
}
    
//overload za konverziju u string
Regex::operator std::string() const {
    return this->reduce();
}

//vraća pojednostavljeni regex
std::string Regex::reduce() const {
    std::string a;
    if (regex_type == ATOMIC)
        for (int i=0; i<size; ++i) a += exp[i];
    else {
        bool bracket = !is_str_const && regex_type == HAS_SEPARATOR || kleen;
        if (bracket) a += BRA;
        for (auto it = subdivisions.begin(); it != subdivisions.end(); ++it) {
            if (it != subdivisions.begin() && deliminator()) a += deliminator();
            a += it->reduce();
        }
        if (bracket) a += KET;
    }
    if (kleen) a+= KLEEN;
    return a;
}

//overload za ispis regexa
std::ostream &operator<< (std::ostream& os, const Regex& r) {
    os <<(std::string) r;
    return os;
}

//vraća tip regexa
Regex::Type Regex::type () const {
    return this->regex_type;
}

//vraća ima li regex kleen operator
bool Regex::has_kleen() const {
    return kleen;
}

//funkcije za iteriranje po djeci regexa
std::deque<Regex>::const_iterator Regex::begin () const {
    return this->subdivisions.begin();
}
std::deque<Regex>::const_iterator Regex::end () const {
    return this->subdivisions.end();
}

char Regex::get() const {
    if (type() != ATOMIC) 
        throw std::invalid_argument("Regex must be ATOMIC to call!");
    return exp[0];
}

const Regex& Regex::open (const std::string& name) {
    return saved[name];
}

void Regex::save_as (const std::string& name) {
    saved[name] = *this;
}

std::unordered_map<std::string, Regex> Regex::saved = std::unordered_map<std::string, Regex>();