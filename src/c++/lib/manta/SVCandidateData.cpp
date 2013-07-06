// -*- mode: c++; indent-tabs-mode: nil; -*-
//
// Manta
// Copyright (c) 2013 Illumina, Inc.
//
// This software is provided under the terms and conditions of the
// Illumina Open Source Software License 1.
//
// You should have received a copy of the Illumina Open Source
// Software License 1 along with this program. If not, see
// <https://github.com/downloads/sequencing/licenses/>.
//

///
/// \author Chris Saunders

#include "manta/SVCandidateData.hh"

#include <cassert>



SVCandidateReadPair&
SVCandidateDataGroup::
getReadPair(const pindex_t::key_type& key)
{
    const pindex_t::const_iterator kiter(_pairIndex.find(key));

    if(kiter == _pairIndex.end())
    {
        _pairIndex[key] = _pairs.size();
        _pairs.push_back(SVCandidateReadPair());
        return _pairs.back();
    }
    else
    {
        return _pairs[kiter->second];
    }
}



void
SVCandidateDataGroup::
add(const bam_record& read)
{
    SVCandidateReadPair& pair(getReadPair(read.qname()));

    SVCandidateRead* target_read_ptr(&(pair.read1));
    if(2 == read.read_no())
    {
        target_read_ptr = (&(pair.read2));
    }
    assert(! target_read_ptr->isSet());
    target_read_ptr->bamrec.copy(read);
}