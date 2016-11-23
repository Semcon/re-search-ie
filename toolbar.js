// Author: Sara Amani, Oskar Risberg, Basim Ali
// Copyright (c) 2016, Semcon Sweden AB
// All rights reserved.

// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,  this list of conditions and
//    the following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the Semcon Sweden AB nor the names of its contributors may be used to endorse or
//    promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

(function(){
    var DATA_URL = 'https://api.myjson.com/bins/1bv2i';
    var TIP_URL = 'http://semcon.com/re-search-tip/';

    var shareUrl = 'http://semcon.com/re-search';
    var jsonData;
    var dropdownTerms = [];
    var currentEngine;

    function getSelectList(){
        var selectList = document.createElement( 'select');
        selectList.className = 're-search-select';
        selectList.id = "termList";

        var defaultOption = document.createElement("option");
        defaultOption.value = 'Try Re-search';
        defaultOption.text = 'Try Re-search';
        selectList.add(defaultOption);

        dropdownTerms.sort(function (a, b) {
            return a.localeCompare(b);
        });

        //Create and append the options
        for (var i = 0; i < dropdownTerms.length; i = i + 1 ) {
            var option = document.createElement("option");
            option.value = dropdownTerms[i];
            option.text = dropdownTerms[i];
            selectList.add(option);
        }

        return selectList;
    }

    function getShare(){
        var shareWrapper = document.createElement( 'div' );
        shareWrapper.className = 're-search-share-wrapper';

        var shareButton = document.createElement( 'a' );
        shareButton.className = 're-search-share-button';
        shareButton.innerText = 'Share';

        shareWrapper.appendChild( shareButton );

        var shareLinkedin = document.createElement( 'a' );
        shareLinkedin.setAttribute( 'href', 'https://www.linkedin.com/shareArticle?url=' + shareUrl + '&title=Example' );
        shareLinkedin.className = 're-search-share-linkedin re-search-hidden';
        shareLinkedin.setAttribute( 'target', '_BLANK' );

        var shareLinkedinImage = document.createElement( 'img' );
        shareLinkedinImage.src = 'http://i.imgur.com/LDWUmYB.png';
        shareLinkedinImage.className = 're-search-share-icon';

        shareLinkedin.appendChild( shareLinkedinImage );

        shareWrapper.appendChild( shareLinkedin );

        var shareFacebook = document.createElement( 'a' );
        shareFacebook.setAttribute( 'href', 'https://www.facebook.com/sharer.php?u=' + shareUrl );
        shareFacebook.className = 're-search-share-facebook re-search-hidden';
        shareFacebook.setAttribute( 'target', '_BLANK' );

        var shareFacebookImage = document.createElement( 'img' );
        shareFacebookImage.src = 'http://i.imgur.com/by3Nifg.png';
        shareFacebookImage.className = 're-search-share-icon';

        shareFacebook.appendChild( shareFacebookImage );

        shareWrapper.appendChild( shareFacebook );

        var shareTwitter = document.createElement( 'a' );
        shareTwitter.setAttribute( 'href', ' https://twitter.com/intent/tweet?url=' + shareUrl + '&text=Example' );
        shareTwitter.className = 're-search-share-twitter re-search-hidden';
        shareTwitter.setAttribute( 'target', '_BLANK' );

        var shareTwitterImage = document.createElement( 'img' );
        shareTwitterImage.src = 'http://i.imgur.com/W4gdDed.png';
        shareTwitterImage.className = 're-search-share-icon';

        shareTwitter.appendChild( shareTwitterImage );

        shareWrapper.appendChild( shareTwitter );

        return shareWrapper;
    }

    function getEngine(){
        if( typeof jsonData === 'undefined' ) {
            return false;
        }

        for( var i = 0; i < jsonData.engines.length; i = i + 1 ){
            var matchCount = 0;

            // Loop over all required matches for the engine
            for( var matchIndex = 0; matchIndex < jsonData.engines[ i ].match.length; matchIndex = matchIndex + 1 ){
                if( window.location.href.indexOf( jsonData.engines[ i ].match[ matchIndex ] ) > -1 ){
                    // We have a match, increment our counter
                    matchCount = matchCount + 1;
                }
            }

            // If we have the same number of matches as required matches we have a valid site
            if( matchCount === jsonData.engines[ i ].match.length ){
                var terms = Object.keys( jsonData.terms[ jsonData.engines[ i ].terms ] );
                for( var x = 0; x < terms.length; x = x + 1 ){
                    if( jsonData.terms[ jsonData.engines[ i ].terms ][ terms[ x ] ].dropdown ){
                        dropdownTerms.push( terms[ x ] );
                    }
                }

                currentEngine = jsonData.engines[ i ];

                return jsonData.engines[ i ];
            }
        }

        return false;
    }

    function getToolbar(){
        var toolbar = document.createElement( 'div' );
        toolbar.className = 're-search-toolbar';
        toolbar.id = 're-search-toolbar';

        var logoWrapper = document.createElement( 'div' );
        logoWrapper.className = 're-search-logo-wrapper';

        var logo = document.createElement( 'img' );
        logo.src = 'http://i.imgur.com/C1flvC7.png';

        logoWrapper.appendChild( logo );

        toolbar.appendChild( logoWrapper );

        var selectArrow = document.createElement( 'div' );
        selectArrow.className = 're-search-arrow-down';

        toolbar.appendChild( selectArrow );

        var selectList = getSelectList();
        toolbar.appendChild( selectList );

        var tipButton = document.createElement( 'button' );
        tipButton.className = 're-search-button re-search-tip-button';
        tipButton.innerText = 'Add to Re-Search';

        toolbar.appendChild( tipButton );

        var approvedTipText = document.createElement( 'div' );
        approvedTipText.className = 're-search-approved-tip-text re-search-hidden';
        approvedTipText.innerText = 'Thumbs up! We\'ll look into that.';

        toolbar.appendChild( approvedTipText );

        var tipText = document.createElement( 'div' );
        tipText.className = 're-search-tip-text re-search-hidden';
        tipText.innerText = 'Do you want to add ';

        var tipTerm = document.createElement( 'span' );
        tipTerm.className = 're-search-tip-term';

        tipText.appendChild( tipTerm );

        toolbar.appendChild( tipText );

        var approveTipButton = document.createElement( 'button' );
        approveTipButton.className = 're-search-button re-search-approve-tip-button re-search-hidden';
        approveTipButton.innerText = 'Yes';

        toolbar.appendChild( approveTipButton );

        var denyTipButton = document.createElement( 'button' );
        denyTipButton.className = 're-search-button re-search-deny-tip-button re-search-hidden';
        denyTipButton.innerText = 'No';

        toolbar.appendChild( denyTipButton );

        var readMoreButton = document.createElement( 'a' );
        readMoreButton.className = 're-search-read-more-button';
        readMoreButton.innerText = 'Read more';
        readMoreButton.href = shareUrl;

        toolbar.appendChild( readMoreButton );

        toolbar.appendChild( getShare() );

        return toolbar;
    }

    function sendTip(){
        var xhr = new XMLHttpRequest();
        xhr.open( 'POST', TIP_URL, true );
        xhr.setRequestHeader( 'Content-Type', 'application/x-www-form-urlencoded' );
        xhr.onreadystatechange = function() {
            if ( xhr.readyState === 4 && xhr.status === 200 ) {
                console.log( 'Tip sent' );
            }
        }

        xhr.send( 'term=' + encodeURIComponent( getCurrentTerm() ).replace( /%20/g, '+' ) );
    }

    function approveTip(){
        document.querySelector( '.re-search-tip-text' ).classList.add( 're-search-hidden' );
        document.querySelector( '.re-search-approve-tip-button' ).classList.add( 're-search-hidden' );
        document.querySelector( '.re-search-deny-tip-button' ).classList.add( 're-search-hidden' );

        document.querySelector( '.re-search-approved-tip-text' ).classList.remove( 're-search-hidden' );
        document.querySelector( '.re-search-select' ).classList.remove( 're-search-hidden' );
        document.querySelector( '.re-search-share-wrapper' ).classList.remove( 're-search-hidden' );
        document.querySelector( '.re-search-read-more-button' ).classList.remove( 're-search-hidden' );
        document.querySelector( '.re-search-arrow-down' ).classList.remove( 're-search-hidden' );

        sendTip();
    }

    function showShareButtons(){
        document.querySelector( '.re-search-share-button' ).classList.add( 're-search-hidden' );

        document.querySelector( '.re-search-share-twitter' ).classList.remove( 're-search-hidden' );
        document.querySelector( '.re-search-share-facebook' ).classList.remove( 're-search-hidden' );
        document.querySelector( '.re-search-share-linkedin' ).classList.remove( 're-search-hidden' );
    }

    function hideTip(){
        document.querySelector( '.re-search-tip-button' ).classList.remove( 're-search-hidden' );
        document.querySelector( '.re-search-select' ).classList.remove( 're-search-hidden' );
        document.querySelector( '.re-search-share-wrapper' ).classList.remove( 're-search-hidden' );
        document.querySelector( '.re-search-read-more-button' ).classList.remove( 're-search-hidden' );
        document.querySelector( '.re-search-arrow-down' ).classList.remove( 're-search-hidden' );

        document.querySelector( '.re-search-tip-text' ).classList.add( 're-search-hidden' );
        document.querySelector( '.re-search-approve-tip-button' ).classList.add( 're-search-hidden' );
        document.querySelector( '.re-search-deny-tip-button' ).classList.add( 're-search-hidden' );
    }

    function getCurrentTerm(){
        return document.querySelector( currentEngine.selectors.input ).value;
    }

    function updateURL( term ){
        window.location.href = currentEngine.url + term;
    }

    function showTip(){
        document.querySelector( '.re-search-tip-button' ).classList.add( 're-search-hidden' );
        document.querySelector( '.re-search-select' ).classList.add( 're-search-hidden' );
        document.querySelector( '.re-search-share-wrapper' ).classList.add( 're-search-hidden' );
        document.querySelector( '.re-search-read-more-button' ).classList.add( 're-search-hidden' );
        document.querySelector( '.re-search-arrow-down' ).classList.add( 're-search-hidden' );

        document.querySelector( '.re-search-tip-text' ).classList.remove( 're-search-hidden' );
        document.querySelector( '.re-search-approve-tip-button' ).classList.remove( 're-search-hidden' );
        document.querySelector( '.re-search-deny-tip-button' ).classList.remove( 're-search-hidden' );

        document.querySelector( '.re-search-tip-term' ).innerText = getCurrentTerm();
    }

    function addListeners(){
        window.addEventListener( 'change', function(event){
            if( event.target.id === 'termList' ){
                var term = document.getElementById( 'termList' ).value;

                updateURL( term );
            }
        });

        window.addEventListener( 'click', function( event ){
            if( event.target.classList.contains( 're-search-tip-button' ) ){
                showTip();
            }
        });

        window.addEventListener( 'click', function( event ){
            if( event.target.classList.contains( 're-search-deny-tip-button' ) ){
                hideTip();
            }
        });

        window.addEventListener( 'click', function( event ){
            if( event.target.classList.contains( 're-search-approve-tip-button' ) ){
                approveTip();
            }
        });

        window.addEventListener( 'click', function( event ){
            if( event.target.classList.contains( 're-search-share-button' ) ){
                event.preventDefault();
                showShareButtons();
            }
        });
    }

    function injectToolbar(){
        if( document.getElementById( 're-search-toolbar' ) ){
            return false;
        }

        var toolbar = getToolbar();
        var body = document.querySelectorAll( 'body' )[ 0 ];
        var currentStyle;
        var newStyle;

        for( var i = 0; i < body.children.length; i = i + 1 ){
            currentStyle = body.children[ i ].getAttribute( 'style' );

            if( !currentStyle ){
                newStyle = 'transform: translateY( 60px );';
            } else {
                newStyle = currentStyle + '; transform: translateY( 60px );';
            }

            body.children[ i ].setAttribute( 'style', newStyle );
        }

        addListeners();
        body.insertBefore( toolbar, body.children[ 0 ] );
    }

    var xhr = new XMLHttpRequest();
    xhr.open( 'GET', DATA_URL, true );
    xhr.onreadystatechange = function() {
        if ( xhr.readyState === 4 && xhr.status === 200 ) {
            jsonData = JSON.parse( xhr.responseText );

            if( getEngine() ){
                injectToolbar();
            }
        }
    }
    xhr.send();
})();
