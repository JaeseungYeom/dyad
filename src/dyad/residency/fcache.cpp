#include <dyad/residency/fcache.hpp>
#include <dyad/utils/murmur3.h>
#include <typeinfo>
#include <iostream>

#define DYAD_UTIL_LOGGER
#include <dyad/common/dyad_logging.h>


namespace dyad_residency {

std::string id_str (const std::string& id)
{
    return id;
}

template <typename T>
std::string id_str (const T& id)
{
    using namespace std;
    return to_string (id);
}


//=============================================================================
//                          Associative Cache Set
//=============================================================================

template <typename IDT>
bool Set_LRU<IDT>::lookup (const IDT& fname, id_iterator_t &it)
{
    id_idx_t& index_id = boost::multi_index::get<id> (m_block_set);
    it = index_id.find (fname);
    return (it != index_id.end ());
}

template <typename IDT>
void Set_LRU<IDT>::evict (void)
{ // LRU
    if (m_block_set.size () == 0) return;
    priority_idx_t& index_priority = boost::multi_index::get<priority> (m_block_set);
    priority_iterator_t it = index_priority.begin ();
    DYAD_LOG_INFO (NULL, "    %s evicts %s from set %u\n", \
                   m_level.c_str (), id_str (it->m_id).c_str (), m_id);
    index_priority.erase (it);
}

template <typename IDT>
void Set_LRU<IDT>::load_and_access (const IDT& fname)
{
    m_num_miss++;

    DYAD_LOG_INFO (NULL, "    %s adds %s to set %u\n", \
                   m_level.c_str (), id_str (fname).c_str (), m_id);
    if (m_size == m_block_set.size ()) {
        evict ();
    }

    m_block_set.insert (Simple_Block<IDT> (fname));
    m_seqno++;
}

template <typename IDT>
void Set_LRU<IDT>::access (id_iterator_t &it)
{
    Simple_Block<IDT> blk = *it;
    m_block_set.erase (it);
    m_block_set.insert (blk);
    m_seqno++;
}

template <typename IDT>
bool Set_LRU<IDT>::access (const IDT& fname)
{
    id_iterator_t it;
    if (lookup (fname, it)) { // hit
        DYAD_LOG_INFO (NULL, "    %s reuses %s from set %u\n", \
                       m_level.c_str (), id_str (fname).c_str (), m_id);
        access (it);
        return true;
    } else { // miss
        load_and_access (fname);
        return false;
    }
}

template <typename IDT>
unsigned int Set_LRU<IDT>::get_priority (unsigned int)
{
    return m_seqno;
}

template <typename IDT>
std::ostream& Set_LRU<IDT>::print (std::ostream &os) const
{
    os << "size         : " << m_size << std::endl;
    os << "num accesses : " << m_seqno<< std::endl;
    os << "num misses   : " << m_num_miss << std::endl;
    os << "blkId        : " << std::endl;

    const priority_idx_t& index_priority = boost::multi_index::get<priority> (m_block_set);
    priority_citerator_t it = index_priority.begin ();
    priority_citerator_t itend = index_priority.end ();

    for (; it != itend; it++) {
        os << it->m_id << std::endl;
    }
    return os;
}

template <typename IDT>
std::ostream& operator<<(std::ostream& os, const Set_LRU<IDT>& cc)
{
    return cc.print (os);
}



template <typename IDT, typename PRT>
bool Set_Prioritized<IDT, PRT>::lookup (const IDT& fname, id_iterator_t &it)
{
    id_idx_t& index_id = boost::multi_index::get<id> (m_block_set);
    it = index_id.find (fname);
    return (it != index_id.end ());
}

template <typename IDT, typename PRT>
void Set_Prioritized<IDT, PRT>::evict (void)
{
    if (m_block_set.size () == 0) return;
    priority_idx_t& index_priority = boost::multi_index::get<priority> (m_block_set);
    priority_iterator_t it = index_priority.begin ();
    DYAD_LOG_INFO (NULL, "    %s evicts %s from set %u\n", \
                   m_level.c_str (), id_str (it->m_id).c_str (), m_id);
    index_priority.erase (it);
}

template <typename IDT, typename PRT>
void Set_Prioritized<IDT, PRT>::load_and_access (const IDT& fname)
{
    m_num_miss++;

    DYAD_LOG_INFO (NULL, "    %s adds %s to set %u\n", \
                   m_level.c_str (), id_str (fname).c_str (), m_id);
    if (m_size == m_block_set.size ()) {
        evict ();
    }

    m_block_set.insert (Ranked_Block<IDT, PRT> (fname, get_priority (PRT ())));
    m_seqno++;
}

template <typename IDT, typename PRT>
void Set_Prioritized<IDT, PRT>::access (id_iterator_t &it)
{
    Ranked_Block<IDT, PRT> blk = *it;
    // reassigning the priority
    blk.m_priority = get_priority (PRT ());
    m_block_set.erase (it);
    m_block_set.insert (blk);
    m_seqno++;
}

template <typename IDT, typename PRT>
bool Set_Prioritized<IDT, PRT>::access (const IDT& fname)
{
    id_iterator_t it;
    if (lookup (fname, it)) { // hit
        DYAD_LOG_INFO (NULL, "   %s reuses %s from set %u\n", \
                       m_level.c_str (), id_str (fname).c_str (), m_id);
        access (it);
        return true;
    } else { // miss
        load_and_access (fname);
        return false;
    }
}

template <typename IDT, typename PRT>
PRT Set_Prioritized<IDT, PRT>::get_priority (PRT)
{
    return m_seqno;
}

template <typename IDT, typename PRT>
std::ostream& Set_Prioritized<IDT, PRT>::print (std::ostream &os) const
{
    os << "size         : " << m_size << std::endl;
    os << "num accesses : " << m_seqno<< std::endl;
    os << "num misses   : " << m_num_miss << std::endl;
    os << "priority blkId:" << std::endl;

    const priority_idx_t& index_priority = boost::multi_index::get<priority> (m_block_set);
    priority_citerator_t it = index_priority.begin ();
    priority_citerator_t itend = index_priority.end ();

    for (; it != itend; it++) {
        os << it->m_priority << ", " << it->m_id << std::endl;
    }
    return os;
}

template <typename IDT, typename PRT>
std::ostream& operator<<(std::ostream& os, const Set_Prioritized<IDT, PRT>& cc)
{
    return cc.print (os);
}


namespace {
template <typename T>
void __attribute__ ((unused)) instantiate_LRU ()
{
    Set_LRU<T> set_lru (1u, 1u, 0u);
    T id;
    set_lru.size ();
    set_lru.get_num_access ();
    set_lru.get_num_miss ();
    set_lru.reset_cnts ();
    set_lru.get_level ();
    set_lru.set_level ("na");
    set_lru.access (id);
    set_lru.print (std::cout);
    std::cout << set_lru;
}

template <typename T, typename P>
void __attribute__ ((unused)) instantiate_Prioritized ()
{
    Set_Prioritized<T, P> set_prt (1u, 1u, 0u);
    T id;
    set_prt.size ();
    set_prt.get_num_access ();
    set_prt.get_num_miss ();
    set_prt.reset_cnts ();
    set_prt.get_level ();
    set_prt.set_level ("na");
    set_prt.access (id);
    set_prt.print (std::cout);
    std::cout << set_prt;
}

void __attribute__ ((unused)) instantiate_all ()
{
    instantiate_LRU<unsigned int> ();
    instantiate_LRU<int> ();
    instantiate_LRU<std::string> ();

    instantiate_Prioritized<unsigned int, unsigned> ();
    instantiate_Prioritized<int, unsigned> ();
    instantiate_Prioritized<std::string, unsigned> ();

    instantiate_Prioritized<unsigned int, float> ();
    instantiate_Prioritized<int, float> ();
    instantiate_Prioritized<std::string, float> ();
}

} // end of namespace

} // end of namespace dyad_residency
