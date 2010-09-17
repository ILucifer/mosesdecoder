#include "Scorer.h"
#include "ScorerFactory.h"
#include "InterpolatedScorer.h"

using namespace std;


InterpolatedScorer::InterpolatedScorer (const string& name, const string& config): Scorer(name,config) {
    //configure regularisation
    static string KEY_WEIGHTS = "weights";
    static string KEY_TYPE = "regtype";
    static string KEY_WINDOW = "regwin";
    static string KEY_CASE = "case";
    static string TYPE_NONE = "none";
    static string TYPE_AVERAGE = "average";
    static string TYPE_MINIMUM = "min";
    static string TRUE = "true";
    static string FALSE = "false";

    string type = getConfig(KEY_TYPE,TYPE_NONE);
    if (type == TYPE_NONE) {
        _regularisationStrategy = REG_NONE;
    } else if (type == TYPE_AVERAGE) {
        _regularisationStrategy = REG_AVERAGE;
    } else if (type == TYPE_MINIMUM) {
        _regularisationStrategy = REG_MINIMUM;
    } else {
        throw runtime_error("Unknown scorer regularisation strategy: " + type);
    }
    cerr << "Using scorer regularisation strategy: " << type << endl;

    string window = getConfig(KEY_WINDOW,"0");
    _regularisationWindow = atoi(window.c_str());
    cerr << "Using scorer regularisation window: " << _regularisationWindow << endl;

    string preservecase = getConfig(KEY_CASE,TRUE);
    if (preservecase == TRUE) {
        _preserveCase = true;
    }else if (preservecase == FALSE) {
        _preserveCase = false;
    }
    cerr << "Using case preservation: " << _preserveCase << endl;

    // name would be: HAMMING,BLEU or similar

    string scorers = name;
    while (scorers.length() > 0) {
        string scorertype = "";
        getNextPound(scorers,scorertype,",");
        ScorerFactory SF;
        Scorer *theScorer=SF.getScorer(scorertype,config);
        _scorers.push_back(theScorer);
    }
    if (_scorers.size() == 0) {
        throw runtime_error("There are no scorers");
    }
    cout << "Number of scorers: " << _scorers.size() << endl;

    //TODO debug this
    string wtype = getConfig(KEY_WEIGHTS,"");
    //Default weights set to uniform ie. if two weights 0.5 each
    //weights should add to 1
    if (wtype.length() == 0) {
        float weight = 1.0/_scorers.size() ;
        //cout << " Default weights:" << weight << endl;        
        for (size_t i = 0; i < _scorers.size(); i ++) {
          _scorerWeights.push_back(weight);
        }
    }else{
        float tot=0;
        //cout << "Defined weights:"  << endl;        
        while (wtype.length() > 0) {
            string scoreweight = "";
            getNextPound(wtype,scoreweight,"+");
            float weight = atof(scoreweight.c_str());
            _scorerWeights.push_back(weight);
            tot += weight;
            //cout << " :" << weight ;        
        }
        //cout << endl;
        if (tot != float(1)) {
            throw runtime_error("The interpolated scorers weights do not sum to 1");
        }
    }
    cout << "The weights for the interpolated scorers are: " << endl;
    for (vector<float>::iterator it = _scorerWeights.begin(); it < _scorerWeights.end(); it++) {
        cout << *it << " " ;
    }
    cout <<endl;


}

void InterpolatedScorer::setScoreData(ScoreData* data) {
   size_t last = 0;
    _scoreData = data;
   for (vector<Scorer*>::iterator itsc =  _scorers.begin(); itsc!=_scorers.end();itsc++){
       int numScoresScorer = (*itsc)->NumberOfScores();
       ScoreData* newData =new ScoreData(**itsc);
       for (size_t i = 0; i < data->size(); i++){
         ScoreArray scoreArray = data->get(i); 
         ScoreArray newScoreArray;
         std::string istr;
         std::stringstream out;
         out << i;
         istr = out.str();
         size_t numNBest = scoreArray.size();
         //cout << " Datasize " << data->size() <<  " NumNBest " << numNBest << endl ; 
         for (size_t j = 0; j < numNBest ; j++){
           ScoreStats scoreStats = data->get(i, j); 
           //cout << "Scorestats " << scoreStats << " i " << i << " j " << j << endl; 
           ScoreStats newScoreStats;
           for (size_t k = last; k < size_t(numScoresScorer + last); k++) {
             ScoreStatsType score = scoreStats.get(k);
             newScoreStats.add(score);
           }
           //cout << " last " << last << " NumScores " << numScoresScorer << "newScorestats " << newScoreStats << endl; 
           newScoreArray.add(newScoreStats);
         }
         newScoreArray.setIndex(istr);
         newData->add(newScoreArray);
       }
       //newData->dump();
       (*itsc)->setScoreData(newData);
       last += numScoresScorer;
   }
}


/** The interpolated scorer calls a vector of scorers and combines them with 
    weights **/
void  InterpolatedScorer::score(const candidates_t& candidates, const diffs_t& diffs,
            statscores_t& scores) {

   //cout << "*******InterpolatedScorer::score" << endl;
   size_t scorerNum = 0;
   for (vector<Scorer*>::iterator itsc =  _scorers.begin(); itsc!=_scorers.end();itsc++){
       int numScores = (*itsc)->NumberOfScores();
       statscores_t tscores;
       (*itsc)->score(candidates,diffs,tscores);
       size_t inc = 0;
       for (statscores_t::iterator itstatsc =  tscores.begin(); itstatsc!=tscores.end();itstatsc++){
           //cout << "Scores " << (*itstatsc) << endl;
           float weight = _scorerWeights[scorerNum];
           if (weight == 0) {
             stringstream msg;
             msg << "No weights for scorer" << scorerNum ;
             throw runtime_error(msg.str());
           }
           if (scorerNum == 0) {
               scores.push_back(weight * (*itstatsc));
           } else {
               scores[inc] +=  weight * (*itstatsc);
           }
           //cout << "Scorer:" << scorerNum <<  " scoreNum:" << inc << " score: " << (*itstatsc) << " weight:" << weight << endl; 
           inc++;
         
       }
       scorerNum++;
   }

}

void InterpolatedScorer::setReferenceFiles(const vector<string>& referenceFiles) {
    for (vector<Scorer *>::iterator itsc =  _scorers.begin(); itsc!=_scorers.end();itsc++){
        //the scorers that use alignments use the reference files in the constructor through config
        (*itsc)->setReferenceFiles(referenceFiles);
    }
}

// Text can be:
// Reference sentence ||| Reference sentence alignment information (as given by MOSES -include-alignment-in-n-best)
// If a permutation distance scorer, send alignment info
// Else if other scorer, remove the alignment info and then send reference as usual
void InterpolatedScorer::prepareStats(size_t sid, const string& text, ScoreStats& entry) {
    stringstream buff;
    string align = text;
    string sentence = "";
    size_t alignmentData = text.find("|||");
    //Get sentence and alignment parts
    if(alignmentData != string::npos) {
        getNextPound(align,sentence, "|||");
    } 
    int i=0;
    for (vector<Scorer*>::iterator itsc =  _scorers.begin(); itsc!=_scorers.end();itsc++){
        ScoreStats tempEntry;
        if ((*itsc)->useAlignment()) {
            (*itsc)->prepareStats(sid, text, tempEntry);
        } else {
            (*itsc)->prepareStats(sid, sentence, tempEntry);
        }
        if (i > 0) buff <<  " ";
        buff << tempEntry;
        i++;
    }
    //cout << " Scores for interpolated: " << buff << endl;
    string str = buff.str();
    entry.set(str);
}
