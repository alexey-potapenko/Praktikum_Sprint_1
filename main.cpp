#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        ++document_count_;
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            // word_to_documents_[word].insert(document_id);
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        // documents_.push_back({document_id, words});
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };
    
    struct Query {
        set<string> minus_words_;
        set<string> plus_words_;
    };
    
    set<string> stop_words_;
    // map<string, set<int>> word_to_documents_;
    map<string, map<int, double>> word_to_document_freqs_; // TF
    int document_count_ = 0;
    
    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }
    
    
    QueryWord ParseQueryWord (string text) const {
        bool is_minus = false;
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        return {text, is_minus, IsStopWord(text)};
    }

    Query ParseQuery(const string& text) const {
        Query query_words;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query_words.minus_words_.insert(query_word.data);
                } else {
                    query_words.plus_words_.insert(query_word.data);
                }
            }
        }
            return query_words;
    }

    vector<Document> FindAllDocuments(const Query& query_words) const {
        vector<Document> matched_documents;
        map<int, double> document_to_relevance;
        // double inverse_document_freq;
        
        for (const string& word : query_words.plus_words_) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            } 
                const double inverse_document_freq = CalculateIDF(word);
            
            for (const auto& [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                // ++document_to_relevance[document_id];
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
        for (const string& word : query_words.minus_words_) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto& [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }
        for (const auto &[document_id, relevance] : document_to_relevance) {
            matched_documents.push_back({document_id, relevance});
        }
        return matched_documents;
    }
    
    double CalculateIDF (const string& word) const {
        return log(document_count_ * 1.0 / word_to_document_freqs_.at(word).size());
    }
    
   
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}
