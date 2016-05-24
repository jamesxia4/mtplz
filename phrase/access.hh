#pragma once

#include "phrase/types.hh"
#include "util/layout.hh"

#include <boost/optional.hpp> // Or C++17

#include <limits>

namespace util { class scoped_memory; }

/* Accessors to interpret a target phrase pointer */
namespace phrase {

class FieldConfig {
  public:
    void Save(util::scoped_memory &mem);
    void Restore(const util::scoped_memory &mem);

    bool words = true;
    std::size_t dense_features = std::numeric_limits<std::size_t>::max();
    bool sparse_features = false;
    std::size_t lexical_reordering = std::numeric_limits<std::size_t>::max();
};

template <class Field> class OptionalField {
  public:
    OptionalField(util::Layout &layout, bool present) {
      if (present) {
        field_ = Field(layout);
      }
    }

    OptionalField(util::Layout &layout, std::size_t size) {
      if (size != std::numeric_limits<std::size_t>::max()) {
        field_ = Field(layout, size);
      }
    }

    auto operator()(const Phrase *phr) const -> typename std::result_of<Field(const void*)>::type {
      return (*field_)(phr);
    }
    template <class Alloc> auto operator()(Phrase *phr, Alloc &alloc) const -> typename std::result_of<Field(void*, Alloc &)>::type {
      return (*field_)(phr, alloc);
    }
    auto operator()(Phrase *phr) const -> typename std::result_of<Field(void*)>::type {
      return (*field_)(phr);
    }
    operator bool() const { return field_; }

  private: 
    boost::optional<Field> field_;
};

// Usage:
// To check if lexicalized reordering is in the phrase table:
//   if (access.lexicalized_reordering)
// Once you're sure it's there, access:
//   access.lexicalized_reordering(phrase);
class Access {
  private:
    util::Layout layout_;

  public:
    explicit Access(const FieldConfig &config) :
      words(layout_, config.words),
      dense_features(layout_, config.dense_features),
      sparse_features(layout_, config.sparse_features),
      lexical_reordering(layout_, config.lexical_reordering) {}

    OptionalField<util::VectorField<WordIndex, VectorSize> > words;
    OptionalField<util::ArrayField<float> > dense_features;
    OptionalField<util::VectorField<SparseFeature, VectorSize> > sparse_features;
    OptionalField<util::ArrayField<float> > lexical_reordering;
    // TODO word alignment, properties?

    // Get the pointer to the next phrase.
    const Phrase *End(const Phrase *phrase) const {
      // TODO optimize.
      // TODO this assumes there's at least one VectorField.
      const uint8_t *base = reinterpret_cast<const uint8_t*>(phrase);
      const uint8_t *end = base + layout_.OffsetsEnd() + 
        *(reinterpret_cast<const VectorSize*>(base + layout_.OffsetsEnd()) - 1);
      return reinterpret_cast<const Phrase*>(end);
    }
};

} // namespace phrase