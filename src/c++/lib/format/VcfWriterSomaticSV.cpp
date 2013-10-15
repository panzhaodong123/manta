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
// <https://github.com/sequencing/licenses/>
//

///
/// \author Chris Saunders
///

#include "format/VcfWriterSomaticSV.hh"

#include "boost/algorithm/string/join.hpp"



void
VcfWriterSomaticSV::
addHeaderFormatSampleKey() const
{
    /// TODO: get sample name from bam header/user
    _os << "FORMAT\tNORMAL\tTUMOR";
}



void
VcfWriterSomaticSV::
addHeaderInfo() const
{
    _os << "##INFO=<ID=SOMATIC,Number=0,Type=Flag,Description=\"Somatic mutation\">\n";
    _os << "##INFO=<ID=SOMATICSCORE,Number=1,Type=Integer,Description=\"Somatic variant Quality score\">\n";
}



void
VcfWriterSomaticSV::
addHeaderFormat() const
{
    _os << "##FORMAT=<ID=PAIR,Number=.,Type=Integer,Description=\"Spanning paired-read support for the ref and alt alleles in the order listed\">\n";
    _os << "##FORMAT=<ID=SR,Number=.,Type=Integer,Description=\"Split read counts for the ref and alt alleles in the order listed\">\n";
    _os << "##FORMAT=<ID=SREV,Number=.,Type=Float,Description=\"Split read evidence for the ref and alt alleles in the order listed\">\n";
}



void
VcfWriterSomaticSV::
addHeaderFilters() const
{
    if (_isMaxDepthFilter)
    {
        _os << "##FILTER=<ID=" << _somaticOpt.maxDepthFilterLabel << ",Description=\"Normal sample site depth is greater than " << _somaticOpt.maxDepthFactor << "x the mean chromosome depth near one or both variant breakends\">\n";
    }
}



void
VcfWriterSomaticSV::
modifyInfo(
    std::vector<std::string>& infotags) const
{
    assert(_modelScorePtr != NULL);
    const SVModelScoreInfo& modelScoreInfo(*_modelScorePtr);

    infotags.push_back("SOMATIC");
    infotags.push_back( str(boost::format("SOMATICSCORE=%i") % modelScoreInfo.somatic.somaticScore) );
}



void
VcfWriterSomaticSV::
modifyTranslocInfo(
    const bool isFirstOfPair,
    std::vector<std::string>& infotags) const
{
    assert(_modelScorePtr != NULL);
    const SVScoreInfo& baseInfo(_modelScorePtr->base);

#if 0
    infotags.push_back( str(boost::format("NORMAL_ALT_BND_PAIR_COUNT=%i") %
                            (isFirstOfPair ? baseInfo.normal.alt.bp1SpanReadCount : baseInfo.normal.alt.bp2SpanReadCount) ) );
    infotags.push_back( str(boost::format("TUMOR_ALT_BND_PAIR_COUNT=%i") %
                            (isFirstOfPair ? baseInfo.tumor.alt.bp1SpanReadCount : baseInfo.tumor.alt.bp2SpanReadCount) ) );
#endif
    infotags.push_back( str(boost::format("BND_DEPTH=%i") %
                            (isFirstOfPair ? baseInfo.bp1MaxDepth : baseInfo.bp2MaxDepth) ) );
    infotags.push_back( str(boost::format("MATE_BND_DEPTH=%i") %
                            (isFirstOfPair ? baseInfo.bp2MaxDepth : baseInfo.bp1MaxDepth) ) );

}



void
VcfWriterSomaticSV::
modifySample(
    const SVCandidate& sv,
    SampleTag_t& sampletags) const
{
    assert(_modelScorePtr != NULL);
    const SVModelScoreInfo& modelScoreInfo(*_modelScorePtr);
    const SVScoreInfo& baseInfo(modelScoreInfo.base);

    std::vector<std::string> values(2);

    static const std::string pairTag("PAIR");
    values[0] = str( boost::format("%i,%i") % baseInfo.normal.ref.spanPairCount % baseInfo.normal.alt.spanPairCount);
    values[1] = str( boost::format("%i,%i") % baseInfo.tumor.ref.spanPairCount % baseInfo.tumor.alt.spanPairCount);
    sampletags.push_back(std::make_pair(pairTag,values));

    if (sv.isImprecise()) return;

    static const std::string srTag("SR");
    values[0] = str( boost::format("%i,%i") % baseInfo.normal.ref.splitReadCount % baseInfo.normal.alt.splitReadCount);
    values[1] = str( boost::format("%i,%i") % baseInfo.tumor.ref.splitReadCount % baseInfo.tumor.alt.splitReadCount);
    sampletags.push_back(std::make_pair(srTag,values));

    static const std::string srevTag("SREV");
    values[0] = str( boost::format("%.1f,%.1f") % baseInfo.normal.ref.splitReadEvidence % baseInfo.normal.alt.splitReadEvidence);
    values[1] = str( boost::format("%.1f,%.1f") % baseInfo.tumor.ref.splitReadEvidence % baseInfo.tumor.alt.splitReadEvidence);
    sampletags.push_back(std::make_pair(srevTag,values));
}



void
VcfWriterSomaticSV::
writeFilter() const
{
    assert(_modelScorePtr != NULL);
    const SVScoreInfo& baseInfo(_modelScorePtr->base);

    writeFilters(baseInfo.filters);
}



void
VcfWriterSomaticSV::
writeSV(
    const EdgeInfo& edge,
    const SVCandidateSetData& svData,
    const SVCandidateAssemblyData& adata,
    const SVCandidate& sv,
    const SVModelScoreInfo& modelScore)
{
    //TODO: this is a lame way to customize subclass behavior:
    _modelScorePtr=&modelScore;
    writeSVCore(edge, svData, adata, sv);
    _modelScorePtr=NULL;
}
