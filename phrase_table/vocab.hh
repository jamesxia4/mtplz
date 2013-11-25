#ifndef PHRASE_TABLE_VOCAB__
#define PHRASE_TABLE_VOCAB__

#include "util/pool.hh"
#include "util/probing_hash_table.hh"
#include "util/string_piece.hh"

#include <stdint.h>

namespace phrase_table {

#pragma pack(push)
#pragma pack(4)
struct VocabInternal {
  typedef uint64_t Key;
  uint64_t GetKey() const { return key; }
  void SetKey(uint64_t to) { key = to; }

  uint64_t key;
  uint32_t id;
};
#pragma pack(pop)
 
class Vocab {
  public:
    typedef uint32_t ID;

    static const ID kUNK = 0;

    Vocab();
    
    uint32_t Find(const StringPiece &str) const;

    ID FindOrInsert(const StringPiece &str);

    StringPiece String(ID id) const {
      return strings_[id];
    }

    // Includes kUNK.
    std::size_t Size() const { return strings_.size(); }
    
  private:
    util::Pool piece_backing_;

    typedef util::AutoProbing<VocabInternal, util::IdentityHash> Map;
    Map map_;

    std::vector<StringPiece> strings_;
};

} // namespace phrase_table
#endif // PHRASE_TABLE_VOCAB__
