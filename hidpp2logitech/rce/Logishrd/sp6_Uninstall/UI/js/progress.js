    //
    // If you switch this to 'step' mode it will attempt to light each LED in sequence.  The graphic must be setup to allow an even step
    // as the top image is clipped to show the bottom image.
    //
    var progressMode='analog';	

    function updateProgress(progressType, progressBar, dispProgressBar, step, total, currentProgress, bUpdatePercent) 
    {
        if (progressBar.width == "")
            progressBar.width = "1%";        

        // NOTE: Setting width to "0%" will produce an "Invalid argument" Javascript error.
        if (step == 0 || total == 0)
            return currentProgress;
        // Make the assumption that nProgressTotal has already been initialized.
        // Be sure to initialize first prior to modifying nProgressStep.

        var barWidth = parseInt(parseInt(step) * 100 / parseInt(total));
        if (progressMode == 'step') {
            var currPix = barWidth * (progressBar.width / 100);
            if (currPix > (currentProgress + step))
                barWidth = (currPix / step) * step;
            else
                return currentProgress;
        }
        currentProgress = barWidth;
        if (barWidth > 0) {
            if (progressMode == 'step')
                barWidth += "px";
            else
                barWidth += "%";

            progressBar.width = barWidth;
            progressBar.filters(0).enabled = 'true';
            if (bUpdatePercent) 
                progress_percent.innerHTML = barWidth;
            
            var testBar1 = document.getElementById(progressType);
            testBar1.style.clip = 'rect(auto,auto,auto,' + barWidth + ')';
            //var testBar2 = document.getElementById('ptest2');
            //testBar2.style.clip = 'rect(auto,auto,auto,' + barWidth + ')';
            //dispProgressBar.style.left = barWidth;
        }
        else {
            progressBar.filters(0).enabled = 'false';
        }
        return currentProgress;
    }
