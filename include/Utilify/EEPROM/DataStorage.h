#pragma once

template <typename DataType>
class DataStorage {
public:
    virtual ~DataStorage() = default; // Ensure proper cleanup in polymorphic use

    // Save the data to the storage medium
    virtual bool save() noexcept = 0;

    // Load the data from the storage medium
    virtual bool load() noexcept = 0;

    // Set the data to be stored
    inline void setData(const DataType& data) noexcept {
        m_data = data;
    }

    // Retrieve the currently stored data
    inline const DataType& data() const noexcept {
        return m_data;
    }

private:
    DataType m_data;
};
